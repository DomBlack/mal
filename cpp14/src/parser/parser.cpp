#include <vector>
#include <sstream>
#include <map>
#include <unordered_map>

#include "parser.h"
#include "util/helpers.h"

using namespace mal;

#define buildASTAtom(T) Right<ast::TypePtr>(std::make_shared<T>(token))
#define buildAST(T, ...) Right<ast::TypePtr>(std::make_shared<T>(token, __VA_ARGS__))

ParseResult mal::readString(const std::string input) {
  auto parser = mal::Parser(input);

  return parser.readForm();
}

static std::unordered_map<std::string, std::string> quotingTypes = {
    { "'",  "quote" },
    { "`",  "quasiquote" },
    { "~",  "unquote" },
    { "~@", "splice-unquote" },
    { "@",  "deref" }
};

ParseResult Parser::readForm() {
  if (tokenizer.peek().eof) {
    return Left<ParseError>(ParseError(tokenizer.peek(), "Unexpected EOF"));
  } else if (tokenizer.peek().value == "(") {
    return this->readList(ast::NodeType::List);
  } else if (tokenizer.peek().value == "[") {
    return this->readList(ast::NodeType::Vector);
  } else if (tokenizer.peek().value == "{") {
    return this->readMap();
  } else if (quotingTypes.find(tokenizer.peek().value) != quotingTypes.end()) {
    return this->readQuotingType();
  } else if (tokenizer.peek().value == "^") {
    return this->readWithMetaData();
  } else {
    return this->readAtom();
  }
}

ParseResult Parser::readList(const ast::NodeType type) {
  const auto openingToken = tokenizer.next();

  std::string finalToken;

  switch (type) {
    case ast::NodeType::List:
      finalToken = ")";
      break;

    case ast::NodeType::Vector:
      finalToken = "]";
      break;

    default:
      return Left<ParseError>(ParseError(openingToken, "readList called for an invalid type"));
  }

  std::vector<ast::TypePtr> items = {};
  while (tokenizer.peek().value != finalToken && !tokenizer.peek().eof) {
    auto result = this->readForm();

    if (result) {
      items.push_back(result.right);
    } else {
      return result;
    }
  }

  auto lastToken = tokenizer.next();

  if (lastToken.eof) {
    return Left<ParseError>(ParseError(lastToken, "Unexpected EOF while reading list"));
  }

  items.shrink_to_fit();

  return Right<ast::TypePtr>(std::make_shared<ast::List>(type, openingToken, items));
}

ParseResult Parser::readMap() {
  const auto token = tokenizer.next();
  std::map<std::string, ast::TypePtr> map = {};

  while (tokenizer.peek().value != "}" && !tokenizer.peek().eof) {
    auto key = this->readForm();

    if (!key) {
      return key;
    }

    if (key.right->type != ast::NodeType::String && key.right->type != ast::NodeType::Keyword) {
      return Left<ParseError>(ParseError(key.right, "Expected Keyword or String as map key"));
    }

    if (tokenizer.peek().eof) {
      break;
    }

    auto value = this->readForm();

    if (!value) {
      return value;
    }

    map[key.right->toString()] = value.right;
  }

  auto lastToken = tokenizer.next();

  if (lastToken.eof) {
    return Left<ParseError>(ParseError(lastToken, "Unexpected EOF while reading map"));
  }

  return buildAST(ast::Map, map);
}

ParseResult Parser::readAtom() {
  auto token = tokenizer.next();

  if (token.value.empty()) {
    return Left<ParseError>(ParseError(token, "Empty token encountered"));

  } else if (isIntegerString(token.value)) {
    return buildAST(ast::Integer, std::atoi(token.value.c_str()));

  } else if (token.value[0] == ';') {
    return buildASTAtom(ast::Comment);

  } else if (token.value == "nil" || token.value == "NIL") {
    return buildASTAtom(ast::Nil);

  } else if (token.value == "true" || token.value == "TRUE") {
    return buildAST(ast::Boolean, true);

  } else if (token.value == "false" || token.value == "FALSE") {
    return buildAST(ast::Boolean, false);

  } else if (token.value[0] == ':') {
    return buildAST(ast::Keyword, token.value.substr(1));

  } else if (token.value[0] == '"') {
    if (token.eof) { // Unfinished string
      return Left<ParseError>(ParseError(token, "Unexpected EOF while reading string"));
    } else {
      return buildAST(ast::String, token.value.substr(1, token.value.length() - 2));
    }

  } else {
    return buildAST(ast::Symbol, token.value);
  }
}

ParseResult Parser::readQuotingType() {
  auto token = tokenizer.next();

  // Read the child and parse it's result down
  auto child = this->readForm();
  if (!child) {
    return child;
  }

  auto symbol = std::make_shared<ast::Symbol>(token, quotingTypes[token.value]);

  std::vector<ast::TypePtr> items = { symbol, child.right };

  return Right<ast::TypePtr>(std::make_shared<ast::List>(ast::NodeType::List, token, items));
}

ParseResult Parser::readWithMetaData() {
  auto token = tokenizer.next();

  // Read the children and parse it's result down
  auto child1 = this->readForm();
  if (!child1) {
    return child1;
  }

  auto child2 = this->readForm();
  if (!child2) {
    return child2;
  }

  auto symbol = std::make_shared<ast::Symbol>(token, "with-meta");

  std::vector<ast::TypePtr> items = { symbol, child2.right, child1.right };

  return Right<ast::TypePtr>(std::make_shared<ast::List>(ast::NodeType::List, token, items));
}

const std::string ParseError::error() const {
  std::stringstream stream;

  stream << _error << " at " << line << ":" << position;

  return stream.str();
}
