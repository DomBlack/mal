#include "tokenizer.h"

std::ostream &operator<< (std::ostream &os, mal::Token const &m) {
  if (m.eof) {
    os << "EOF @ " << m.line << ":" << m.position;
  } else {
    os << "\"" << m.value << "\" @ " << m.line << ":" << m.position;
  }
  return os;
}


mal::Tokenizer::Tokenizer(const std::string input): input(input), length(input.size()) {
  // Read the first token so it can be peeked at
  this->next();
}

mal::Token mal::Tokenizer::peek() {
  return this->nextToken;
}

mal::Token mal::Tokenizer::next() {
  const Token current = this->nextToken;

  if (current.eof) {
    return current;
  }

  // Have we reached EOF?
  if (this->index >= this->input.size()) {
    this->nextToken = Token(this->line, this->position);
  } else {
    // Loop over white space and commas
    if (!this->readWhiteSpaceOrCommas()) {
      // EOF found?
      this->nextToken = Token(this->line, this->position);
    } else {
      // No, let's parse the token
      auto startingLine = this->line;
      auto startingPos = this->position;

      char character;
      this->nextChar(character);

      if (character == '~' && this->peekChar() == '@') {
        // Check for "~@"
        this->nextChar(character); // skip the @
        this->nextToken = Token("~@", startingLine, startingPos);

      } else if (this->isSpecialSingleChar(character)) {
        std::string str = "";
        str += character;

        // Is it one of the special characters?
        this->nextToken = Token(str, startingLine, startingPos);

      } else if (character == '"') {
        // Read a string
        std::string str = "";
        str += character;

        bool previousWasEscape = false;
        bool stringEndFound = false;

        while (this->nextChar(character)) {
          if (!previousWasEscape) {
            previousWasEscape = character == '\\';
            if (!previousWasEscape) {
              // Append to string
              str += character;

              // Stop looping for end of string
              if (character == '"') {
                stringEndFound = true;
                break;
              }
            }

          } else {
            previousWasEscape = false;

            switch (character) {
              case '\\':
              case '"':
                str += character;
                break;
              case 'n':
                str += '\n';
                break;
              default:
                break;
            }
          }
        }

        if (stringEndFound) {
          this->nextToken = Token(str, startingLine, startingPos);
        } else {
          this->nextToken = Token("\"", startingLine, startingPos);
          this->nextToken.eof = true;
        }

      } else if (character == ';') {
        //Read a comment until EOF
        std::string str(&character);

        while (this->nextChar(character)) {
          str += character;
        }

        this->nextToken = Token(str, startingLine, startingPos);
      } else {
        // Read any string token
        std::string str = "";
        str += character;
        char next = this->peekChar();

        while (!this->isWhiteSpaceOrComma(next) && !this->isSpecialSingleChar(next) && !this->isNotSymbolChar(next)) {
          if (!this->nextChar(character)) {
            break;
          }

          str += character;
          next = this->peekChar();
        }

        this->nextToken = Token(str, startingLine, startingPos);
      }
    }
  }

  return current;
}

char mal::Tokenizer::peekChar() {
  if (this->index < this->length) {
    return this->input[this->index];
  } else {
    return 0;
  }
}

bool mal::Tokenizer::nextChar(char &character) {
  if (this->index >= this->length) {
    character = 0;
    return false;
  } else {
    character = this->input[this->index++];

    if (character == '\n') {
      this->line++;
      this->position = 1;
    } else {
      this->position++;
    }

    return true;
  }
}

bool mal::Tokenizer::readWhiteSpaceOrCommas() {
  auto character = this->peekChar();

  while (this->isWhiteSpaceOrComma(character) || character == 0) {
    // Read over it and check for EOF
    if (!this->nextChar(character)) {
      return false;
    }

    character = this->peekChar();
  }

  return true;
}

bool mal::Tokenizer::isWhiteSpaceOrComma(char character) {
  return character == ' ' || character == '\t' || character == '\n' || character == ',';
}

bool mal::Tokenizer::isSpecialSingleChar(char character) {
  return character == '[' || character == ']'
         || character == '{' || character == '}'
         || character == '(' || character == ')'
         || character == '\'' || character == '`' || character == '~' || character == '^' || character == '@';
}

bool mal::Tokenizer::isNotSymbolChar(char character) {
  return character == '"' || character == ';';
}
