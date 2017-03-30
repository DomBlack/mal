#pragma once

#include <string>
#include <iostream>

namespace mal {
  struct Token {
    std::string value;
    unsigned int line;
    unsigned int position;
    bool eof;

    Token(std::string value, unsigned int line, unsigned int position): value(value), line(line), position(position), eof(false) {};
    Token(unsigned int line, unsigned int position): value(""), line(line), position(position), eof(true) {};
  };

  class Tokenizer {
    private:
      /// The source input
      const std::string input;

      /// The sources length
      const std::string::size_type length;

      /// Our index within input
      std::string::size_type index = 0;

      /// The line number we are on
      unsigned int line     = 1;

      /// The position on the line we are on
      unsigned int position = 1;

      /// The next token
      Token nextToken = Token("", 0, 0);

      /**
       *
       * @param character The character read
       *
       * @return `true` if we did not reach EOF
       */
      bool nextChar(char &character);

      /// Peeks at the next character or returns \0
      char peekChar();

      /// Reads over all whitespace or commas
      bool readWhiteSpaceOrCommas();

      /// Is the character a whitespace character?
      bool isWhiteSpaceOrComma(char character);

      /// Is the character a special character?
      bool isSpecialSingleChar(char character);

      /// Is this a special character we should ignore?
      bool isNotSymbolChar(char character);

    public:
      Tokenizer(const std::string input);

      Token peek();
      Token next();
  };
}

std::ostream &operator<< (std::ostream &os, mal::Token const &m);
