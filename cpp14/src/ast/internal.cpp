#include "internal.h"

#include <ostream>

#include "ast/types.h"
#include "runtime/helpers.h"

using namespace mal;

void internal::InternalFunc::toStream(std::ostream &stream) const {
  stream << this->name;
}

namespace mal {
  namespace internal {
    ast::TypePtr addFunc = LAMBDA_AS_AST_NODE(
        "+",
        [](auto callsite, auto funcName, FuncArgs arguments) -> EvalResult {
          ARG_COUNT(2);
          ARG_CAST(lhs, 1, Integer);
          ARG_CAST(rhs, 2, Integer);

          rtnResult(Integer, lhs->value + rhs->value);
        }
    );

    ast::TypePtr subFunc = LAMBDA_AS_AST_NODE(
        "-",
        [](auto callsite, auto funcName, FuncArgs arguments) -> EvalResult {
          ARG_COUNT(2);
          ARG_CAST(lhs, 1, Integer);
          ARG_CAST(rhs, 2, Integer);

          rtnResult(Integer, lhs->value - rhs->value);
        }
    );

    ast::TypePtr mulFunc = LAMBDA_AS_AST_NODE(
        "*",
        [](auto callsite, auto funcName, FuncArgs arguments) -> EvalResult {
          ARG_COUNT(2);
          ARG_CAST(lhs, 1, Integer);
          ARG_CAST(rhs, 2, Integer);

          rtnResult(Integer, lhs->value * rhs->value);
        }
    );

    ast::TypePtr divFunc = LAMBDA_AS_AST_NODE(
        "/",
        [](auto callsite, auto funcName, FuncArgs arguments) -> EvalResult {
          ARG_COUNT(2);
          ARG_CAST(lhs, 1, Integer);
          ARG_CAST(rhs, 2, Integer);

          rtnResult(Integer, lhs->value / rhs->value);
        }
    );
  }
}
