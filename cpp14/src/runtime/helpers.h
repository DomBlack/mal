#pragma once

#define rtnErrorAt(site, reason) return Left<ParseError>(ParseError(site, reason))
#define RTN_ERROR(reason) rtnErrorAt(callsite, reason)
#define rtnResult(T, value) return Right<ast::TypePtr>(std::make_shared<ast::T>(callsite->toDummyToken(), value))

#define ARG_COUNT(number) if (arguments.size() != number) { RTN_ERROR("Expected " #number " arguments got " + std::to_string(arguments.size()) + " for function `" + funcName + "`"); }
#define ARG_CAST(name, index, T) std::shared_ptr<ast::T> name = (arguments[index - 1]->type == ast::NodeType::T) ? std::static_pointer_cast<ast::T>(arguments[index - 1]) : nullptr; \
  if (!name) { \
    rtnErrorAt(arguments[index - 1], "Expected argument " #index " to be a " #T); \
  }

#define ARG_CAST_TO_LIST(name, index) std::shared_ptr<ast::List> name = (arguments[index - 1]->type == ast::NodeType::List || arguments[index - 1]->type == ast::NodeType::Vector) ? std::static_pointer_cast<ast::List>(arguments[index - 1]) : nullptr; \
  if (!name) { \
    rtnErrorAt(arguments[index - 1], "Expected argument " #index " to be a list or vector"); \
  }

#define CAST_NODE(name, node, T) std::shared_ptr<ast::T> name = (node->type == ast::NodeType::T) ? std::static_pointer_cast<ast::T>(node) : nullptr; \
  if (!name) { \
    rtnErrorAt(node, "Expected argument " #name " to be a " #T); \
  }

#define ARG_EVAL(name, index, env) const auto result_##name = eval(arguments.at(index - 1), env); \
  if (!result_##name) { \
    return result_##name.asLeft(); \
  } \
  const auto name = result_##name.right;

#define EVAL_NODE(name, node, env) const auto result_##name = eval(node, env); \
  if (!result_##name) { \
    return result_##name.asLeft(); \
  } \
  const auto name = result_##name.right;

/// Defines a given lambda as an AST node
#define LAMBDA_AS_AST_NODE(name, lambda, token) std::make_shared<mal::internal::InternalFunc>(name, lambda, token)

#define DEF_SPECIALS(name, nameStr, code) EvalResult name(const ast::TypePtr callsite, const internal::FuncArgs arguments, const runtime::EnvPtr env) { \
  const std::string funcName = nameStr; \
  code \
}

#define RTN_VALUE(value) return Right<ast::TypePtr>(value)

#define RTN_NODE(T, value) return Right<ast::TypePtr>(std::make_shared<ast::T>(callsite->toDummyToken(), value))

#define RTN_NIL() return Right<ast::TypePtr>(std::make_shared<ast::Nil>(callsite->toDummyToken()))
