#pragma once

#include <runtime/environment.h>
#include "types.h"

namespace mal {
  namespace ast {
    class UserFunc : public Type, public std::enable_shared_from_this<UserFunc> {
      private:
        /// Is this function a macro?
        bool _isMacro = false;

      public:
        /// The function body
        const TypePtr ast;

        /// The parameter names
        const std::vector<std::string> params;

        const std::size_t paramCount;

        /// The runtime enviroment which we closed over when creating the function
        const runtime::EnvPtr env;

        /// The internal function original returned by the fn* lambda
        const std::shared_ptr<internal::InternalFunc> fn;

        UserFunc(
            const Token token,
            const TypePtr ast,
            const std::vector<std::string> params,
            const std::size_t paramCount,
            const runtime::EnvPtr env,
            const std::shared_ptr<internal::InternalFunc> fn
        ): ast::Type(ast::NodeType::UserFunc, token), ast(ast), params(params), paramCount(paramCount), env(env), fn(fn) {};

        void toStream(std::ostream &stream) const override;

        /// Returns a copy of this function as a macro
        ast::TypePtr asMacro() {
          if (this->_isMacro) {
            return shared_from_this();
          } else {
            auto copy = std::make_shared<UserFunc>(toDummyToken(), ast, params, paramCount, env, fn);
            copy->_isMacro = true;
            return copy;
          }
        };

        bool isMacro() {
          return this->_isMacro;
        }
    };
  }
}
