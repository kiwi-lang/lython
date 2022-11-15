
#ifndef LYTHON_TREE_EVAL_HEADER
#define LYTHON_TREE_EVAL_HEADER

#include "ast/magic.h"
#include "ast/ops.h"
#include "ast/visitor.h"
#include "sema/bindings.h"
#include "sema/builtin.h"
#include "sema/errors.h"
#include "utilities/strings.h"

namespace lython {

using PartialResult = Node;

struct TreeEvaluatorTrait {
    using StmtRet = PartialResult*;
    using ExprRet = PartialResult*;
    using ModRet  = PartialResult*;
    using PatRet  = PartialResult*;
    using Trace   = std::true_type;

    enum
    { MaxRecursionDepth = LY_MAX_VISITOR_RECURSION_DEPTH };
};

struct StackTrace {
    // The statement point to the line
    // while the expression points to a specific location in the line
    StmtNode const* stmt;
    ExprNode const* expr;
};

/* Tree evaluator is a very simple interpreter that is also very slow.
 * It takes as input the binding array generated by the semantic analysis
 * i.e the evaluation context and the expression to evaluate given the context
 *
 * The expression to evaluate is often a call to a function, for a standard
 * program that function will be main.
 *
 * While being slow this evaluator has the advantage or returning an AST
 * as result which makes it perfect for compile-time usage.
 *
 * We can call the evaluator on standard declaration which will result in
 * constant folding everything it can. Note that because we are able to represent
 * complex types at compile time, string/list/dict even object operations can be folded.
 *
 * .. code-block:: python
 *
 *    value = '.'.join(['a', 'b', 'c'])     # <= Everything is known at compile time
 *                                          #    It can be folded
 *
 * Additionally, this can be used to generate code at compile time by
 * creating function with types as arguments.
 *
 * .. code-block::
 *
 *    def Point(type: Type):
 *        class point:
 *            x: type
 *            y: type
 *
 *        return point
 *
 *    Pointi = Point(int)
 *    Pointf = Point(float)                # <= Generate new types at compile time
 *
 *
 * Evaluation Implementation
 * -------------------------
 *
 * 1. Reuse as much as possible from the sema context
 *    Save the context inside each statement so we can use it
 *    during evaluation.
 *    Because the context is copied, it is easy to do parallel executions
 *
 * 2. Create a different context for evaluation only
 */
struct TreeEvaluator: BaseVisitor<TreeEvaluator, false, TreeEvaluatorTrait> {

    public:
    using Super = BaseVisitor<TreeEvaluator, false, TreeEvaluatorTrait>;

    TreeEvaluator(Bindings& bindings): bindings(bindings) { traces.push_back(StackTrace()); }

    virtual ~TreeEvaluator() {}

    template <typename Exception, typename... Args>
    ConstantValue raise(Args... args) {
        return ConstantValue::none();
    }

#define FUNCTION_GEN(name, fun) virtual PartialResult* fun(name##_t* n, int depth);

#define X(name, _)
#define SSECTION(name)
#define MOD(name, fun)
#define EXPR(name, fun)  FUNCTION_GEN(name, fun)
#define STMT(name, fun)  FUNCTION_GEN(name, fun)
#define MATCH(name, fun) FUNCTION_GEN(name, fun)

    NODEKIND_ENUM(X, SSECTION, EXPR, STMT, MOD, MATCH)

#undef X
#undef SSECTION
#undef EXPR
#undef STMT
#undef MOD
#undef MATCH

#undef FUNCTION_GEN

    // this some clean up code, acknowledge we have exceptions
    // but this code needs to run regardless, it will stop if new exceptions are raised
    struct HandleException {
        HandleException(TreeEvaluator* self): self(self) {
            self->handling_exceptions = int(self->exceptions.size());
        }

        ~HandleException() { self->handling_exceptions = 0; }

        TreeEvaluator* self;
    };

    // Helpers
    PartialResult* get_next(Node* iterator, int depth);
    PartialResult* call_enter(Node* ctx, int depth);
    PartialResult* call_exit(Node* ctx, int depth);
    PartialResult* call_native(Call_t* call, BuiltinType_t* n, int depth);
    PartialResult* call_script(Call_t* call, FunctionDef_t* n, int depth);
    PartialResult* call_constructor(Call_t* call, ClassDef_t* cls, int depth);
    PartialResult* make_generator(Call_t* call, FunctionDef_t* n, int depth);

    void execute_body(Array<StmtNode*>& body, int depth);
    void execute_loop_body(Array<StmtNode*>& body, int depth);

    void raise_exception(PartialResult* exception, PartialResult* cause);

    // Only returns true when new exceptions pop up
    // we usually expect 0 exceptions,
    // during exceptions handling we will execpt n and this will only be true
    // if new exceptions are raised during the previous exceptions handling
    bool has_exceptions() const { return int(exceptions.size()) > handling_exceptions; }

    PartialResult* eval(StmtNode_t* stmt);

    Constant* make(ClassDef* class_t, Array<Constant*> args, int depth);

    private:
    PartialResult* exec(StmtNode_t* stmt, int depth) {
        StackTrace& trace = get_trace();
        trace.stmt        = stmt;
        return Super::exec(stmt, depth);
    }

    PartialResult* exec(ExprNode_t* expr, int depth) {
        StackTrace& trace = get_trace();
        trace.expr        = expr;
        return Super::exec(expr, depth);
    }

    StackTrace& get_trace() { return traces[traces.size() - 1]; }

    // private:
    // --------
    public:
    void set_return_value(PartialResult* ret) {
        // I cant delete the return value here, it might be re-used in the context
        // It is hard to decide when to delete the return value
        // the problem lie when a value is returned, its scope ends
        // but the value belongs to the upper scope
        //
        // Maybe just make a stack of scopes and return makes the value belong to the upper scope
        // or promote it to the upper scope so it does not get deleted
        //
        // I thought about allocating the return value before the call is made
        // so I do not have to promote the return value (it would already be on the right scope)
        // but it might get tricky with values referenced twice
        // when the variable is promoted the references are removed but not freed because it is
        // still used as a return value

        // if (return_value != nullptr) {
        //     root.remove_child_if_parent(return_value, true);
        // }
        return_value = ret;
    }

    // This can be used as a root for garbage collection
    // root is never deleted but its children gets checked as reachable or not
    // we can traverse the bindings struct to check if all values are reachable or not
    // every time we leave a scope we could do a quick small GC step on that scope
    // to remove free temporary variables and only keep the return value
    Expression root;

    Bindings&      bindings;
    PartialResult* return_value = nullptr;

    private:
    // `Registers`

    bool loop_break    = false;
    bool loop_continue = false;
    bool yielding      = false;

    PartialResult* cause               = nullptr;
    int            handling_exceptions = 0;

    Array<struct lyException*> exceptions;
    Array<StackTrace>          traces;
};

}  // namespace lython

#endif