#include <string>
#include <iostream>
#include <sstream>

#include "ast/types.h"
#include "parser/parser.h"
#include "util/readline.h"

namespace mal {
  /**
   * Reads the input
   *
   * @param input The input to read
   *
   * @return The input unmodified
   */
  ParseResult read(const std::string input) {
    return readString(input);
  }

  /**
   * Evaluates the input
   *
   * @param input The input to read
   *
   * @return The input unmodified
   */
  ast::TypePtr eval(ast::TypePtr input) {
    return input;
  }

  /**
   * Prints the output
   *
   * @param output The input to read
   *
   * @return The output unmodified
   */
  std::string print(ast::TypePtr output) {
    std::stringstream stream;

    stream << output;

    return stream.str();
  }

  /**
   * Runs the read line
   *
   * @param input The input to read
   *
   * @return The output
   */
  std::string rep(const std::string input) {
    auto ast = read(input);

    if (ast) {
      return print(eval(ast.right));
    } else {
      return ast.left.error();
    }
  }
}

int main() {

  // Create our line reader
  auto readLine = mal::ReadLine("user> ", "~/.mal-history");

  // While we have lines to read (until EOF) execute them in the REPL
  std::string line;
  while (readLine >> line) {
    std::cout << mal::rep(line) << std::endl;
  }

  return 0;
}
