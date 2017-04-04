#include <string>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include "ast/types.h"
#include "parser/parser.h"
#include "util/readline.h"
#include "ast/internal.h"

namespace mal {
  typedef std::unordered_map<std::string, ast::TypePtr> ReplEnv;

  ReplEnv baseReplEnv = {
      { "+", mal::internal::addFunc },
      { "-", mal::internal::subFunc },
      { "*", mal::internal::mulFunc },
      { "/", mal::internal::divFunc }
  };

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

  EvalResult eval(const ast::TypePtr input, const ReplEnv env);

  EvalResult evalAST(const ast::TypePtr input, const ReplEnv env) {
    if (input->type == ast::NodeType::Symbol) {
      // Extract the environment variable
      const auto symbol = std::static_pointer_cast<ast::Symbol>(input);

      if (env.find(symbol->value) == env.end()) {
        return Left<ParseError>(ParseError(input, "Unknown symbol `" + symbol->value + "`"));
      } else {
        return Right<ast::TypePtr>(env.at(symbol->value));
      }

    } else if (input->type == ast::NodeType::List || input->type == ast::NodeType::Vector) {
      // Evaluate all items in the list
      const auto list = std::static_pointer_cast<ast::List>(input);
      std::vector<ast::TypePtr> newList;

      for (const auto item : list->items) {
        const EvalResult result = eval(item, env);

        // If this item didn't evaluate, return it's error
        if (!result) {
          return result.asLeft();
        }

        newList.push_back(result.right);
      }

      newList.shrink_to_fit();

      // Build the new list
      return Right<ast::TypePtr>(std::make_shared<ast::List>(list->type, list->toDummyToken(), newList));

    } else if (input->type == ast::NodeType::Map) {
      // Evaluate all the values in the map
      const auto map = std::static_pointer_cast<ast::Map>(input);
      std::map<std::string, ast::TypePtr> newMap;

      for (const auto item : map->map) {
        const EvalResult result = eval(item.second, env);

        // If this item didn't evaluate, return it's error
        if (!result) {
          return result.asLeft();
        }

        newMap[item.first] = result.right;
      }

      // Build the new map
      return Right<ast::TypePtr>(std::make_shared<ast::Map>(map->toDummyToken(), newMap));

    } else {
      // Nothing to evaluate
      return Right<ast::TypePtr>(input);
    }
  }

  /**
   * Evaluates the input
   *
   * @param input The input to read
   *
   * @return The input unmodified
   */
  EvalResult eval(const ast::TypePtr input, const ReplEnv env) {
    if (input->type == ast::NodeType::List) {
      const auto params =  std::static_pointer_cast<ast::List>(input)->items;

      if (params.empty()) {
        return Right<ast::TypePtr>(input);
      } else {
        const auto result = evalAST(input, env);

        if (!result) {
          return result;
        } else if (result.right->type != ast::NodeType::List) {
          return Left<ParseError>(ParseError(result.right, "Expected a list back after evalAST on a list!"));
        }

        const auto list = std::static_pointer_cast<ast::List>(result.right)->items;
        const auto func = list.front();
        const auto arguments = std::vector<ast::TypePtr>( list.begin() + 1, list.end() );

        if (func->type == ast::NodeType::InternalFunc) {
          const auto f = std::static_pointer_cast<internal::InternalFunc>(func);
          return f->func(input, f->name, arguments);
        } else {
          return Left<ParseError>(ParseError(func, "Unable to execute `" + func->toString() + "` as a function call"));
        }
      }
    } else {
      return evalAST(input, env);
    }
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
    const auto parseResult = read(input);

    if (parseResult) {
      const auto evalResult = eval(parseResult.right, baseReplEnv);

      if (evalResult) {
        return print(evalResult.right);
      } else {
        return evalResult.left.error();
      }
    } else {
      return parseResult.left.error();
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
