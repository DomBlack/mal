#include <string>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include "ast/types.h"
#include "parser/parser.h"
#include "util/readline.h"
#include "ast/internal.h"
#include "ast/userFunc.h"
#include "ast/atom.h"
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
  bool isSymbol(const ast::TypePtr input, const std::string specialName) {
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

  DEF_SPECIALS(fn, "fn*", {
    ARG_COUNT(2);

    // Check the binds are a vecotr
    if (arguments.front()->type != ast::NodeType::Vector && arguments.front()->type != ast::NodeType::List) {
      rtnErrorAt(arguments.front(), "Expected argument 0 to be a Vector");
    }

    // Convert into a vector of strings for the binds
    const auto list  = std::static_pointer_cast<ast::List>(arguments.front())->items;
    const auto count = list.size();

    std::vector<std::string> binds;
    const auto func  = arguments.back();

    // Create the binds list
    bool variadic = false;

    for (std::size_t i = 0; i < count; i++) {
      CAST_NODE(symbol, list.at(i), Symbol);

      if (symbol->value == "&") {
        if ((i + 2) != count) {
          RTN_ERROR("Variadic parameters must be at the end");
        } else {
          variadic = true;
        }
      }

      binds.push_back(symbol->value);
    }

    // -2 because the variadic list might be empty (-1 because of the &)
    const auto finalCount = variadic ? count - 2 : count;


    // Create a lambda which will execute the fn* on call
    const auto lambda = LAMBDA_AS_AST_NODE(
      "#function",
      [=](auto callsite, auto funcName, internal::FuncArgs arguments) -> EvalResult {
        if (arguments.size() < finalCount) {
          RTN_ERROR("Expected " + std::to_string(count) + " arguments got " + std::to_string(arguments.size()) + " for function `" + funcName + "`");
        }

        auto funcEnv = runtime::Environment::create(env, callsite->toDummyToken(), binds, arguments);
        return eval(func, funcEnv);
      },
      callsite->toDummyToken()
    );

    const auto userFunc = std::make_shared<mal::ast::UserFunc>(callsite->toDummyToken(), func, binds, finalCount, env, lambda);
    RTN_VALUE(userFunc);
  });

  /**
   * Helper function
   *
   * @param ast An AST
   *
   * @return If that AST is a non empty list or vector
   */
  bool isPair(const ast::TypePtr ast) {
    if (ast->type == ast::NodeType::List || ast->type == ast::NodeType::Vector) {
      const auto list = std::static_pointer_cast<ast::List>(ast);

      return !list->items.empty();
    }

    return false;
  }

  ast::TypePtr listWithoutHead(const Token callsite, const std::vector<ast::TypePtr> list) {
    std::vector<ast::TypePtr> newList;

    for (std::size_t i = 1; i < list.size(); i++) {
      newList.push_back(list[i]);
    }

    return std::make_shared<ast::List>(ast::NodeType::List, callsite, newList);
  }

  EvalResult quasiquote(const Token callsite, const ast::TypePtr ast) {
    if (!isPair(ast)) {
      std::vector<ast::TypePtr> quasiquoteList =
          { std::make_shared<ast::Symbol>(callsite, "quote"), ast };

      RTN_VALUE(
          std::make_shared<ast::List>(ast::NodeType::List, callsite, quasiquoteList)
      );
    }

    // It has to be a populated list now, because isPair returns false or empty lists or non-lists
    const auto list = std::static_pointer_cast<ast::List>(ast);
    const auto head = list->items.front();

    if (isSymbol(head, "unquote")) {
      if (list->items.size() != 2) {
        RTN_ERROR("unquote expects 1 argument");
      }

      RTN_VALUE(list->items[1]);
    } else if (
        isPair(head) &&
        isSymbol(std::static_pointer_cast<ast::List>(head)->items.front(), "splice-unquote")
    ) {
      const auto headList = std::static_pointer_cast<ast::List>(head)->items;

      if (headList.size() < 2) {
        rtnErrorAt(head->toDummyToken(), "splice-unquote requires 2 arguments");
      }

      const auto result = quasiquote(callsite, listWithoutHead(callsite, list->items));

      if (!result) {
        return result;
      }

      std::vector<ast::TypePtr> quasiquoteList =
          { std::make_shared<ast::Symbol>(callsite, "concat"), headList[1], result.right };

      RTN_VALUE(
          std::make_shared<ast::List>(ast::NodeType::List, callsite, quasiquoteList)
      );
    } else {
      const auto result = quasiquote(callsite, listWithoutHead(callsite, list->items));

      if (!result) {
        return result;
      }

      const auto headResult = quasiquote(callsite, head);

      if (!headResult) {
        return headResult;
      }

      std::vector<ast::TypePtr> quasiquoteList = { std::make_shared<ast::Symbol>(callsite, "cons"), headResult.right, result.right };

      RTN_VALUE(
          std::make_shared<ast::List>(ast::NodeType::List, callsite, quasiquoteList)
      );
    }
  }

  /**
   * Evaluates the input
   *
   * @param input The input to read
   *
   * @return The input unmodified
   */
  EvalResult eval(ast::TypePtr input, runtime::EnvPtr env) {
    while (true) {
      if (input->type == ast::NodeType::List) {
        const auto params = std::static_pointer_cast<ast::List>(input)->items;

        if (params.empty()) {
          RTN_VALUE(input);

        } else if (isSymbol(params.front(), "quote")) {
          if (params.size() != 2) {
            rtnErrorAt(input, "quote expects 1 argument, got " + std::to_string(params.size() - 1));
          }

          RTN_VALUE(params.back());

        } else if (isSymbol(params.front(), "quasiquote")) {
          if (params.size() != 2) {
            rtnErrorAt(input, "quasiquote expects 1 argument, got " + std::to_string(params.size() - 1));
          }

          // TCO quasiquote
          const auto qqResult = quasiquote(input->toDummyToken(), params.back());

          if (!qqResult) {
            return qqResult;
          }

          input = qqResult.right;

        } else if (isSymbol(params.front(), "def!")) {
          return def(input, std::vector<ast::TypePtr>(params.begin() + 1, params.end()), env);

        } else if (isSymbol(params.front(), "let*")) {
          const auto arguments = std::vector<ast::TypePtr>(params.begin() + 1, params.end() - 1);
          if (arguments.size() != 2) {
            rtnErrorAt(input, "let* expects 2 arguments, got " + std::to_string(arguments.size()));
          }

          if (arguments.front()->type != ast::NodeType::List && arguments.front()->type != ast::NodeType::Vector) {
            rtnErrorAt(arguments.front(), "let* expecteds argument 0 to be a List or Vector for");
          }

          const auto list = std::static_pointer_cast<ast::List>(arguments.front())->items;

          const unsigned long size = list.size();
          if (size % 2 == 1) {
            rtnErrorAt(arguments.front(), "let* expects an even number of binding parameters");
          }

          env = runtime::Environment::create(env);

          // Build the bindings
          for (unsigned long i = 0; i < size; i += 2) {
            CAST_NODE(symbol, list.at(i), Symbol);
            EVAL_NODE(value, list.at(i + 1), env);

            env->set(symbol->value, value);
          }

          // Execute the second argument
          input = arguments.back();

        } else if (isSymbol(params.front(), "fn*")) {
          return fn(input, std::vector<ast::TypePtr>(params.begin() + 1, params.end()), env);

        } else if (isSymbol(params.front(), "if")) {
          const auto arguments = std::vector<ast::TypePtr>(params.begin() + 1, params.end());

          // If can be two or three arguments
          if (arguments.size() < 2 || arguments.size() > 3) {
            rtnErrorAt(
                input,
                "Expected 2 or 3 arguments got " + std::to_string(arguments.size()) + " for if statement"
            );
          }

          // evaluate the predicate
          ARG_EVAL(predicate, 1, env);

          // Check if the predicate was false or nil
          bool isFalse = predicate->type == ast::NodeType::Nil;
          if (predicate->type == ast::NodeType::Boolean) {
            CAST_NODE(result, predicate, Boolean);
            isFalse = !result->value;
          }

          if (isFalse) {
            // evaluate the false response if we have one
            if (arguments.size() == 3) {
              input = arguments.at(2); // TCO
            } else {
              // Otherwise return Nil
              return Right<ast::TypePtr>(std::make_shared<ast::Nil>(input->toDummyToken()));
            }
          } else {
            input = arguments.at(1);  // TCO
          }

        } else if (isSymbol(params.front(), "do")) {
          if (params.size() == 0) {
            rtnErrorAt(input, "Nothing passed into do for it to do!");
          }

          const auto arguments = std::vector<ast::TypePtr>(params.begin() + 1, params.end() - 1);
          ast::TypePtr lastResult = nullptr;

          for (const auto arg : arguments) {
            EVAL_NODE(result, arg, env);
            lastResult = result;
          }

          input = params.back(); // TCO

        } else if (isSymbol(params.front(), "swap!")) {
          if (params.size() < 2) {
            rtnErrorAt(input, "swap! expects at least 2 arguments");
          }

          const auto arguments = std::vector<ast::TypePtr>(params.begin() + 1, params.end());
          ARG_EVAL(arg1, 1, env);
          CAST_NODE(atom, arg1, Atom);

          // Grab the rest of the arguments and prep them for a function call
          auto funcCall = std::vector<ast::TypePtr>(params.begin() + 3, params.end());

          // Prefix with "func", "atom value"
          funcCall.insert(funcCall.begin(), arguments[1]);
          funcCall.insert(funcCall.begin() + 1, atom->value);

          EVAL_NODE(
              swap_result,
              std::make_shared<ast::List>(ast::NodeType::List, input->toDummyToken(), funcCall),
              env
          );

          atom->value = swap_result;

          RTN_VALUE(swap_result);
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
          const auto arguments = std::vector<ast::TypePtr>(list.begin() + 1, list.end());

          if (func->type == ast::NodeType::UserFunc) {
            const auto f = std::static_pointer_cast<ast::UserFunc>(func);

            if (arguments.size() < f->paramCount) {
              rtnErrorAt(
                  input,
                  "Expected at least " + std::to_string(f->paramCount) + " arguments got " + std::to_string(arguments.size()) + " for user defined function"
              );
            }

            input = f->ast;
            env   = runtime::Environment::create(env, input->toDummyToken(), f->params, arguments);
          } else if (func->type == ast::NodeType::InternalFunc) {
            const auto f = std::static_pointer_cast<internal::InternalFunc>(func);
            return f->func(input, f->name, arguments);
          } else {
            return Left<ParseError>(
                ParseError(func, "Unable to execute `" + func->toString() + "` as a function call"));
          }
        }
      } else {
        return evalAST(input, env);
      }
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

int main(int argc, char *argv[]) {
  // Create our line reader
  auto readLine = mal::ReadLine("user> ", "~/.mal-history");
  auto env      = mal::runtime::Environment::create();

  using namespace mal;

  // Declare eval
  env->set(
      "eval",
      LAMBDA_AS_AST_NODE(
        "eval",
        [env](internal::CallSite callsite, internal::FuncName funcName, internal::FuncArgs arguments) -> mal::EvalResult {
          ARG_COUNT(1);

          ARG_EVAL(result, 1, env);

          RTN_VALUE(result);
        },
        mal::Token(0, 0)
      )
  );

  // Create the ARGV list
  std::vector<ast::TypePtr> argList;
  for (int i = 2; i < argc; i++) {
    const std::string argument(argv[i]);

    argList.push_back(std::make_shared<ast::String>(Token(0, 0), argument));
  }
  argList.shrink_to_fit();

  env->set(
    "*ARGV*",
    std::make_shared<ast::List>(ast::NodeType::List, Token(0, 0), argList)
  );

  // Declare not in mal
  mal::rep("(def! not (fn* (a) (if a false true)))", env);

  // Declare load-file in mal
  mal::rep("(def! load-file (fn* (f) (eval (read-string (str \"(do \" (slurp f) \")\")))))", env);

  if (argc == 1) {
    // While we have lines to read (until EOF) execute them in the REPL
    std::string line;
    while (readLine >> line) {
      std::cout << mal::rep(line, env) << std::endl;
    }
  } else {
    // Otherwise we are running a mal program
    const std::string file(argv[1]);

    const auto parseResult = read("(load-file \"" + file + "\")");

    if (parseResult) {
      const auto evalResult = eval(parseResult.right, env);

      // Unlike mal::rep we only care about errors for output, if the program has interesting stuff
      // it will use prn or println

      if (!evalResult) {
        std::cerr << evalResult.left.error() << std::endl;
        return 1;
      }
    } else {
      std::cerr << parseResult.left.error() << std::endl;
      return 2;
    }
  }

  return 0;
}
