#include <string>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include "ast/types.h"
#include "parser/parser.h"
#include "util/readline.h"
#include "ast/internal.h"
#include "runtime/environment.h"
#include "runtime/helpers.h"

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

  /// Is the given type a special?
  bool isSpecial(const ast::TypePtr input, const std::string specialName) {
    if (input->type == ast::NodeType::Symbol) {
      return std::static_pointer_cast<ast::Symbol>(input)->value == specialName;
    } else {
      return false;
    }
  }

  EvalResult eval(const ast::TypePtr input, runtime::EnvPtr env);

  EvalResult evalAST(const ast::TypePtr input, const runtime::EnvPtr env) {
    if (input->type == ast::NodeType::Symbol) {
      // Extract the environment variable
      const auto symbol = std::static_pointer_cast<ast::Symbol>(input);
      const auto holdingEnv = env->find(symbol->value);

      if (holdingEnv == nullptr) {
        return Left<ParseError>(ParseError(input, "Unknown symbol `" + symbol->value + "`"));
      } else {
        const auto func = holdingEnv->get(symbol->value);

        if (func == nullptr) {
          return Left<ParseError>(ParseError(input, "Unable to get `" + symbol->value + "` from the environment"));
        } else {
          return Right<ast::TypePtr>(func);
        }
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

  DEF_SPECIALS(def, "def!", {
    ARG_COUNT(2);
    ARG_CAST(symbol, 1, Symbol);
    ARG_EVAL(value,  2, env);

    // Set the enviroment variable
    env->set(symbol->value, value);

    RTN_VALUE(value);
  });

  DEF_SPECIALS(let, "let*", {
    ARG_COUNT(2);

    if (arguments.front()->type != ast::NodeType::List && arguments.front()->type != ast::NodeType::Vector) {
      rtnErrorAt(arguments.front(), "Expected argument 0 to be a List or Vector"); \
    }
    const auto list = std::static_pointer_cast<ast::List>(arguments.front())->items;

    const unsigned long size = list.size();
    if (size % 2 == 1) {
      RTN_ERROR("Expected an even number of binding parameters");
    }

    auto innerEnv = runtime::Environment::create(env);

    // Build the bindings
    for (unsigned long i = 0; i < size; i += 2) {
      CAST_NODE(symbol, list.at(i), Symbol);
      EVAL_NODE(value, list.at(i + 1), innerEnv);

      innerEnv->set(symbol->value, value);
    }

    // Execute the second argument
    ARG_EVAL(result, 2, innerEnv);
    RTN_VALUE(result);
  });

  /**
   * Evaluates the input
   *
   * @param input The input to read
   *
   * @return The input unmodified
   */
  EvalResult eval(const ast::TypePtr input, runtime::EnvPtr env) {
    if (input->type == ast::NodeType::List) {
      const auto params =  std::static_pointer_cast<ast::List>(input)->items;

      if (params.empty()) {
        RTN_VALUE(input);

      } else if (isSpecial(params.front(), "def!")) {
        return def(input, std::vector<ast::TypePtr>(params.begin() + 1, params.end()), env);

      } else if (isSpecial(params.front(), "let*")) {
        return let(input, std::vector<ast::TypePtr>(params.begin() + 1, params.end()), env);

      } else {
        const auto result = evalAST(input, env);

        if (!result) {
          return result;
        } else if (result.right->type != ast::NodeType::List) {
          return Left<ParseError>(ParseError(result.right, "Expected a list back after evalAST on a list!"));
        }

        const auto list = std::static_pointer_cast<ast::List>(result.right)->items;

        if (list.empty()) {
          return Left<ParseError>(ParseError(result.right, "List was empty after evalAST on non empty list!"));
        }

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
  std::string rep(const std::string input, runtime::EnvPtr env) {
    const auto parseResult = read(input);

    if (parseResult) {
      const auto evalResult = eval(parseResult.right, env);

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
  auto env      = mal::runtime::Environment::create();

  // While we have lines to read (until EOF) execute them in the REPL
  std::string line;
  while (readLine >> line) {
    std::cout << mal::rep(line, env) << std::endl;
  }

  return 0;
}
