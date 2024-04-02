#include "dtypes.h"
#include "libtest.h"

namespace lython {

#define GENCASES(name) Array<VMTestCase> const &name##_vm_examples();

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
}