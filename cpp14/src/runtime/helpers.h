#pragma once

#define rtnErrorAt(site, reason) return Left<ParseError>(ParseError(site, reason))
#define RTN_ERROR(reason) rtnErrorAt(callsite, reason)
#define rtnResult(T, value) return Right<ast::TypePtr>(std::make_shared<ast::T>(callsite->toDummyToken(), value))

#define ARG_COUNT(number) if (arguments.size() != 2) { RTN_ERROR("Expected " #number " arguments got " + std::to_string(arguments.size()) + " for function `" + funcName + "`"); }
#define ARG_CAST(name, index, T) std::shared_ptr<ast::T> name = (arguments[index - 1]->type == ast::NodeType::T) ? std::static_pointer_cast<ast::T>(arguments[index - 1]) : nullptr; \
  if (!name) { \
    rtnErrorAt(arguments[index - 1], "Expected argument " #index " to be a " #T); \
  }

#define CAST_NODE(name, node, T) std::shared_ptr<ast::T> name = (node->type == ast::NodeType::T) ? std::static_pointer_cast<ast::T>(node) : nullptr; \
  if (!name) { \
    rtnErrorAt(node, "Expected argument " #name " to be a " #T); \
  }

#define ARG_EVAL(name, index, env) const auto name_result = eval(arguments.at(index - 1), env); \
  if (!name_result) { \
    return name_result.asLeft(); \
  } \
  const auto name = name_result.right;

#define EVAL_NODE(name, node, env) const auto name_result = eval(node, env); \
  if (!name_result) { \
    std::cout << "Error executing " #name " returning left with" << std::endl; \
    return name_result.asLeft(); \
  } \
  const auto name = name_result.right;

/// Defines a given lambda as an AST node
#define LAMBDA_AS_AST_NODE(name, lambda) std::make_shared<mal::internal::InternalFunc>(name, lambda)

#define DEF_SPECIALS(name, nameStr, code) EvalResult name(const ast::TypePtr callsite, const internal::FuncArgs arguments, const runtime::EnvPtr env) { \
  const std::string funcName = nameStr; \
  code \
}

#define RTN_VALUE(value) return Right<ast::TypePtr>(value);
