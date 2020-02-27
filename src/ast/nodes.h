#ifndef LYTHON_AST
#define LYTHON_AST
/*
 *  What is a Program ?
 *
 *      You've got known and unknown. Program use known data to compute
 *      unknown using specified procedures.
 *
 *  For example:
 *      -> runtime known -> Value changes
 *      -> compile known ->
 */


#include <memory>
#include <numeric>
#include <utility>

#include "ast/expressions.h"
#include "ast/names.h"
#include "lexer/token.h"
#include "utilities/stack.h"
#include "interpreter/value.h"


namespace lython {

class Module;

namespace AST {
struct Node {
public:
    NodeKind kind;

    Node(NodeKind k = NodeKind::KUndefined):
        kind(k)
    {}
};

} // namespace AbstractSyntaxTree
namespace AbstractSyntaxTree = AST;
using Attributes = Array<Tuple<StringRef, Expression>>;


namespace AST {
// We declare the leafs of our program
// -----------------------------------

// A Parameter is a special construct that represent a unknown value
// that is unknown at compile time but will be known at runtime
struct Parameter : public Node {
public:
    StringRef  name;
    Expression type;

    Parameter(StringRef name, Expression type)
        : Node(NodeKind::KParameter), name(name), type(type)
    {}

    Parameter(const String &name, Expression type)
        : Parameter(get_string(name), type)
    {}
};

struct pl_hash {
    std::size_t operator()(Parameter &v) const noexcept;
    std::hash<std::size_t> _h;
};


using ParameterList = Array<Parameter>;
using ParameterDict = Dict<StringRef, Parameter, string_ref_hash>;


struct Builtin : public Node {
public:
    StringRef  name;
    Expression type;
    size_t     argument_size;

    Builtin(StringRef name, Expression type, size_t n) :
        Node(NodeKind::KBuiltin), name(name), type(std::move(type)), argument_size(n)
    {}

    Builtin(const String &name, Expression type, size_t n)
        : Builtin(get_string(name), type, n)
    {}
};

struct Arrow : public Node {
public:
    Arrow(): Node(NodeKind::KArrow)
    {}

    ParameterList params;
    Expression return_type;
};

using Variables = Dict<Parameter, Expression, pl_hash>;

struct Type : public Node {
public:
    String name;

    Type(String name) : Node(NodeKind::KType), name(std::move(name)) {}
};

// Math Nodes for ReverPolish parsing
enum class MathKind {
    Operator,
    Value,
    Function,
    VarRef,
    None
};

struct MathNode {
    MathKind kind;
    int arg_count = 1;
    Expression ref;
    String name = "";
};

/**
 * instead of creating a billions expression node we create a single node
 *  that holds all the expressions.
 */
struct ReversePolish: public Node {
public:
    Stack<Expression> stack;

    ReversePolish(Stack<Expression> str)
        : Node(NodeKind::KReversePolish), stack(std::move(str)) {}
};

struct Value : public Node {
public:
    lython::Value value;
    Expression    type;

    template <typename T>
    Value(T val, Expression type)
        : Node(NodeKind::KValue), value(val), type(type) {}

    template<typename V>
    V get_value(){
        return value.get<V>();
    }

    ValueKind get_tag(){
        return value.tag;
    }

    template<typename V, typename T>
    T cast(){
        return T(get_value<V>());
    }
};


// We declare Basic nodes of our program
// -------------------------------------

// A binary operator is a function with two parameters
// Some language specify binary operator as function
// we want our language to be readable
struct BinaryOperator : public Node {
public:
    Expression rhs;
    Expression lhs;
    StringRef op;

    BinaryOperator(Expression lhs, Expression rhs, StringRef op)
        : Node(NodeKind::KBinaryOperator), rhs(std::move(rhs)), lhs(std::move(lhs)), op(op) {}
};

struct UnaryOperator : public Node {
public:
    Expression expr;
    StringRef op;

    UnaryOperator():
        Node(NodeKind::KUnaryOperator)
    {}
};

struct Operator : public Node {
public:
    String name;

    Operator(String op)
        : Node(NodeKind::KOperator), name(std::move(op)) {}
};


struct Call : public Node {
public:
    using Arguments = Array<Expression>;
    using KwArguments = Dict<String, Expression>;

    Expression function;
    // Positional arguments
    Arguments   arguments;
    // Keyword arguments
    KwArguments kwargs;


    Call():
        Node(NodeKind::KCall)
    {}
};

// Block Instruction
// -------------------------------------

// Should I make a sequential + Parallel Intruction Block ?
// similar to let and let* in scheme

// Add get_return_type()
struct SeqBlock : public Node {
public:
    Array<Expression> blocks;

    SeqBlock():
        Node(NodeKind::KSeqBlock)
    {}
};

// Functions
// -------------------------------------

// Functions are Top level expression
struct Function : public Node {
public:
    Expression    body;
    ParameterList args;
    Expression    return_type;
    StringRef     name;
    String        docstring;

    Function(StringRef name)
        : Node(NodeKind::KFunction), name(name)
    {}

    Function(String const &name)
        : Function(get_string(name))
    {}
};

struct ExternFunction: public Node{
    StringRef name;

    ExternFunction(String const &name)
        : Node(NodeKind::KExternFunction), name(get_string(name))
    {}
};

//  This allow me to read an entire file but only process
//  used ens
struct UnparsedBlock : public Node {
public:
    Array<Token> tokens;

    UnparsedBlock() = default;

    UnparsedBlock(Array<Token> &toks)
        : Node(NodeKind::KUnparsedBlock), tokens(toks)
    {}
};

struct Statement : public Node {
public:
    int8        statement;
    Expression  expr;

    Statement(): Node(NodeKind::KStatement)
    {}
};

struct Reference : public Node {
public:
    StringRef   name;
    Expression  type;
    int         index;
    int         length;

    Reference(StringRef name, int loc, int length, Expression type):
        Node(NodeKind::KReference), name(name), type(type), index(loc), length(length)
    {}

    Reference(String const& name, int loc, int length, Expression type):
        Reference(get_string(name), loc, length, type)
    {}
};
using Ref = Reference;

struct Struct : public Node {
public:
    using IndexMapping = Dict<StringRef, int, string_ref_hash>;

    StringRef    name;
    Attributes   attributes;  // Ordered list of attributes
    IndexMapping offset;      // String to int
    String       docstring;

    Struct(StringRef name):
        Node(NodeKind::KStruct), name(name)
    {}

    Struct(String const& name):
        Struct(get_string(name))
    {}

    void insert(String const& attr, Expression expr){
        return insert(get_string(attr), expr);
    }

    void insert(StringRef const& attr, Expression expr){
        offset[attr] = attributes.size();
        attributes.emplace_back(attr, expr);
    }
};

/*
struct QualifiedType : public Node {
public:
    enum TypeSpecifier {
        Void,
        Char,
        Short,
        Int,
        Long,
        Float,
        Double,
        Signed,
        Unsigned,
        UserStruct,
        UserEnum,
        UserUnion,
        UserTypedef
    };
    enum StorageSpecifier { Auto, Register, Static, Extern, Typedef };
    enum TypeQualifier { Const, Volatile };

    String name;
    TypeSpecifier spec_type;
    StorageSpecifier spec_storage;
    TypeQualifier type_qualifier;
};
*/

} // namespace AbstractSyntaxTree
} // namespace lython

#endif