#include <string>
#include <iostream>

#include "util/readline.h"

namespace mal {
  /**
   * Reads the input
   *
   * @param input The input to read
   *
   * @return The input unmodified
   */
  std::string read(std::string input) {
    return input;
  }

  /**
   * Evaluates the input
   *
   * @param input The input to read
   *
   * @return The input unmodified
   */
  std::string eval(std::string input) {
    return input;
  }

  /**
   * Prints the output
   *
   * @param output The input to read
   *
   * @return The output unmodified
   */
  std::string print(std::string output) {
    return output;
  }

  /**
   * Runs the read line
   *
   * @param input The input to read
   *
   * @return The output
   */
  std::string rep(std::string input) {
    return print(eval(read(input)));
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
