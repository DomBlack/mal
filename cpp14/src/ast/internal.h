#pragma once

#include <string>
#include <parser/parser.h>

#include "types.h"
#include "util/either.h"

namespace mal {
  typedef Either<ParseError, ast::TypePtr> EvalResult;

  namespace internal {
    /// The function name as called
    typedef std::string                         FuncName;

    /// The place in the code we are calling from
    typedef ast::TypePtr                        CallSite;

    /// Function arguments
    typedef std::vector<ast::TypePtr>           FuncArgs;

    /// Function lambda
    typedef std::function<EvalResult(CallSite, FuncName, FuncArgs)> FuncLambda;

    class InternalFunc : public ast::Type {
      public:
        std::string name;

        FuncLambda func;

        InternalFunc(const std::string name, const FuncLambda func): ast::Type(ast::NodeType::InternalFunc, Token(0, 0)), name(name), func(func) {};

        void toStream(std::ostream &stream) const override;
    };

    extern ast::TypePtr addFunc;
    extern ast::TypePtr subFunc;
    extern ast::TypePtr mulFunc;
    extern ast::TypePtr divFunc;
  }
}
