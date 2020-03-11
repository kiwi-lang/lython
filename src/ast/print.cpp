#include "print.h"
#include "../parser/module.h"
#include "visitor.h"

namespace lython {

// String to_infix(Stack<AST::MathNode>::ConstIterator &iter, int prev=0);

struct ASTPrinter: public ConstVisitor<ASTPrinter, std::ostream&>{
    std::ostream& out;
    int indent;
    int precedence = -1;

    ASTPrinter(std::ostream& out, int indent = 0):
        out(out), indent(indent)
    {}

    std::ostream& undefined(Node_t, std::size_t){
        return out << "<undefined>";
    }

    std::ostream &parameter(Parameter_t param, std::size_t) {
        return out << param->name;
    }

    std::ostream &unary(UnaryOperator_t un, size_t d) {
        out << un->op;
        out << ' ';
        visit(un->expr, d);
        return out;
    }

    int get_precedence(Expression const& node){
        if (node.kind() == AST::NodeKind::KBinaryOperator){
            return node.ref<AST::BinaryOperator>()->precedence;
        }
        if (node.kind() == AST::NodeKind::KUnaryOperator){
            return node.ref<AST::UnaryOperator>()->precedence;
        }
        return -1;
    }

    std::ostream &binary(BinaryOperator_t bin, std::size_t d) {
        bool parens = false;

        int prev = precedence;
        parens = prev >= bin->precedence;
        precedence = bin->precedence;

        if (parens)
            out << '(';

        visit(bin->lhs, d);

        String const& str = bin->op.str();

        if (str == "."){
            out << bin->op.str();
        } else {
            out << " " << bin->op.str() << " ";
        }

        visit(bin->rhs, d);

        if (parens)
            out << ')';

        precedence = prev;
        return out;
    }

    std::ostream &sequential(SeqBlock_t blocks, std::size_t d) {
        for (auto i = 0ul; i + 1 < blocks->blocks.size(); ++i) {
            out << std::string(std::size_t(indent) * 4, ' ');
            visit(blocks->blocks[i], d);
            out << '\n';
        }

        out << std::string(std::size_t(indent) * 4, ' ');
        visit(*(blocks->blocks.end() - 1), d);
        return out;
    }

    std::ostream &unparsed(UnparsedBlock_t blocks, std::size_t) {
        for (auto &tok : blocks->tokens)
            tok.print(out, indent);
        return out;
    }

    std::ostream &statement(Statement_t stmt, std::size_t d) {
        out << keyword_as_string()[stmt->statement] << " ";
        visit(stmt->expr, d);
        return out;
    }

    std::ostream &reference(Reference_t ref, std::size_t) {
        return out << ref->name; // << "[" << ref->index << "]";
    }

    std::ostream &builtin(Builtin_t blt, int32) {
        return out << blt->name;
    }

    std::ostream &type(Type_t type, std::size_t) {
        return out << type->name;
    }

    std::ostream &value(Value_t val, std::size_t d) {
        val->value.print(out);
        if (false && val->type){
            out << ": ";
            visit(val->type, d);
        }
        return out;
    }

    std::ostream &struct_type(Struct_t cstruct, std::size_t d) {
        out << "struct " << cstruct->name << ":\n";
        std::string indentation = std::string(std::size_t((indent + 1) * 4), ' ');

        if (cstruct->docstring.size() > 0) {
            out << indentation << "\"\"\"" << cstruct->docstring << "\"\"\"\n";
        }

        //for (auto &item : cstruct->attributes) {
        for (auto i = 0ul; i + 1 < cstruct->attributes.size(); ++i) {
            auto const& item = cstruct->attributes[i];
            out << indentation << std::get<0>(item) << ": ";
            visit(std::get<1>(item), d);
            out << "\n";
        }

        auto const& item = *(cstruct->attributes.end() - 1);
        out << indentation << std::get<0>(item) << ": ";
        visit(std::get<1>(item), d);
        return out;
    }

    std::ostream &call(Call_t call, std::size_t d) {
        call->function.print(out, indent);
        out << "(";

        int32 n = int32(call->arguments.size()) - 1;
        for (int i = 0; i < n; ++i) {
            visit(call->arguments[std::size_t(i)], d);
            out << ", ";
        }

        if (n >= 0) {
            visit(call->arguments[std::size_t(n)], d);
        }

        return out << ")";
    }

    std::ostream &arrow(Arrow_t aw, std::size_t d) {
        int n = int(aw->params.size()) - 1;

        out << "(";

        for (int i = 0; i < n; ++i) {
            visit(&aw->params[size_t(i)], d);
            out << ", ";
        }

        if (n >= 0) {
            visit(&aw->params[size_t(n)], d);
        }

        out << ") -> ";
        out << aw->return_type;

        aw->return_type.print(out, indent);
        return out;
    }

    std::ostream& operator_fun(Operator_t op, std::size_t d){
        return out << op->name;
    }

    std::ostream &function(Function_t fun, std::size_t d) {
        // debug("Function Print");

        out << "def " << fun->name << "(";

        for (uint32 i = 0, n = uint32(fun->args.size()); i < n; ++i) {
            out << fun->args[i].name;

            if (fun->args[i].type) {
                out << ": ";
                visit(fun->args[i].type, d);
            }

            if (i < n - 1)
                out << ", ";
        }

        if (fun->return_type) {
            out << ") -> ";
            visit(fun->return_type, d);
            out << ":\n";
        } else
            out << "):\n";

        std::string indentation = std::string(size_t(indent + 1) * 4, ' ');

        if (fun->docstring.size() > 0) {
            out << indentation << "\"\"\"" << fun->docstring << "\"\"\"\n";
        }

        indent += 1;
        visit(fun->body, d);
        indent -= 1;
        return out;
    }
};

std::ostream& print(std::ostream& out, const Expression expr, int indent){
    return ASTPrinter(out, indent).visit(expr);
}
}
