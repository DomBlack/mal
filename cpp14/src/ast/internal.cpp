#include "internal.h"

#include <ostream>

#include "ast/types.h"

using namespace mal;

#define rtnErrorAt(site, reason) return Left<ParseError>(ParseError(site, reason))
#define rtnError(reason) rtnErrorAt(callsite, reason)
#define rtnResult(T, value) return Right<ast::TypePtr>(std::make_shared<ast::T>(callsite->toDummyToken(), value))

#define ARG_COUNT(number) if (arguments.size() != 2) { rtnError("Expected " #number " arguments got " + std::to_string(arguments.size()) + " for function `" + funcName + "`"); }
#define ARG_CAST(name, index, T) std::shared_ptr<ast::T> name = (arguments[index - 1]->type == ast::NodeType::T) ? std::static_pointer_cast<ast::T>(arguments[index - 1]) : nullptr; \
  if (!name) { \
    rtnErrorAt(arguments[index - 1], "Expected argument " #index " to be a " #T); \
  }

void internal::InternalFunc::toStream(std::ostream &stream) const {
  stream << this->name;
}

namespace mal {
  namespace internal {
    ast::TypePtr addFunc = std::make_shared<InternalFunc>(
        "+",
        [](auto callsite, auto funcName, FuncArgs arguments) -> EvalResult {
          ARG_COUNT(2);
          ARG_CAST(lhs, 1, Integer);
          ARG_CAST(rhs, 2, Integer);

          rtnResult(Integer, lhs->value + rhs->value);
        }
    );

    ast::TypePtr subFunc = std::make_shared<InternalFunc>(
        "-",
        [](auto callsite, auto funcName, FuncArgs arguments) -> EvalResult {
          ARG_COUNT(2);
          ARG_CAST(lhs, 1, Integer);
          ARG_CAST(rhs, 2, Integer);

          rtnResult(Integer, lhs->value - rhs->value);
        }
    );

    ast::TypePtr mulFunc = std::make_shared<InternalFunc>(
        "*",
        [](auto callsite, auto funcName, FuncArgs arguments) -> EvalResult {
          ARG_COUNT(2);
          ARG_CAST(lhs, 1, Integer);
          ARG_CAST(rhs, 2, Integer);

          rtnResult(Integer, lhs->value * rhs->value);
        }
    );

    ast::TypePtr divFunc = std::make_shared<InternalFunc>(
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
