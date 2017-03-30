#pragma once

#include <string>
#include <memory>

#include "ast/types.h"
#include "util/either.h"
#include "tokenizer.h"

namespace mal {
  class ParseError
  {
    private:
      unsigned int line;
      unsigned int position;
      std::string _error;

    public:
      ParseError(const Token token, const std::string error): line(token.line), position(token.position), _error(error) {};
      ParseError(const ast::TypePtr node, const std::string error): line(node->line), position(node->position), _error(error) {};

      const std::string error() const;
  };

  typedef Either<ParseError, ast::TypePtr> ParseResult;

  ParseResult readString(const std::string input);

  class Parser {
    private:
      Tokenizer tokenizer;

      ParseResult readList(const ast::NodeType type);
      ParseResult readMap();
      ParseResult readAtom();
      ParseResult readQuotingType();
      ParseResult readWithMetaData();

    public:
      Parser(const std::string input): tokenizer(input) {};

      ParseResult readForm();
  };
}
