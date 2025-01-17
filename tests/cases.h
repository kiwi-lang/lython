#ifndef LYTHON_TESTS_CASES_HEADER
#define LYTHON_TESTS_CASES_HEADER

#include "ast/nodes.h"
#include "dtypes.h"
#include "parser/parsing_error.h"

#include <catch2/catch.hpp>
#include <sstream>

using namespace lython;

struct TestCase {
    TestCase(String const &c, Array<String> const &u = Array<String>(), String const &t = ""):
        code(c), errors(u), expected_type(t) {}

    String        code;
    Array<String> errors;
    String        expected_type;
};

#define GENCASES(name) Array<TestCase> const &name##_examples();

#define X(name, _)
#define SSECTION(name)
#define EXPR(name, _) GENCASES(name)
#define STMT(name, _) GENCASES(name)
#define MOD(name, _)
#define MATCH(name, _)

NODEKIND_ENUM(X, SSECTION, EXPR, STMT, MOD, MATCH)

#undef X
#undef SSECTION
#undef EXPR
#undef STMT
#undef MOD
#undef MATCH

#undef GENTEST

String NE(String const &name);

String NC(std::string const &name);

String TE(String const &lhs_v, String const &lhs_t, String const &rhs_v, String const &rhs_t);

String AE(String const &name, String const &attr);

String UO(String const &op, String const &lhs, String const &rhs);

String IE(String const &import, String const &name);

String MNFE(String const &module);

#endif