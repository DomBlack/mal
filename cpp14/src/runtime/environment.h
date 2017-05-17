#pragma once

#include <unordered_map>
#include <array>

#include "ast/types.h"
#include "ast/internal.h"
#include "core.h"
#include "helpers.h"

namespace mal {
  namespace runtime {
    class Environment;

    typedef std::shared_ptr<Environment> EnvPtr;

    class Environment : public std::enable_shared_from_this<Environment> {
      private:
        const EnvPtr outer;

        std::unordered_map<std::string, ast::TypePtr> data = {};

        /// Creates the root environment
        Environment(): outer(nullptr) {
          for (const auto pair : mal::runtime::ns) {
            data[pair.first] = LAMBDA_AS_AST_NODE(pair.first, pair.second, mal::Token(0, 0));
          }
        };

        /// Creates an inner environment
        Environment(const EnvPtr outer): outer(outer) {};

        /// Creates a function scoped environment
        Environment(const EnvPtr outer, const Token position, const std::vector<std::string> binds, const std::vector<ast::TypePtr> values): outer(outer) {
          auto size = std::min(binds.size(), values.size());

          // Account for an empty binds
          if (size == binds.size() - 2 && binds[size] == "&") {
            size++;
          }

          for (std::size_t i = 0; i < size; i++) {
            if (binds[i] == "&" && (i + 1) < binds.size()) {
              if (values.size() == i) {
                // There is no data for this variadic call
                data[binds[i + 1]] = std::make_shared<ast::List>(
                    ast::NodeType::List, position,
                    std::vector<ast::TypePtr>()
                );
              } else {
                // If this is a variadic bind, then do it
                data[binds[i + 1]] = std::make_shared<ast::List>(
                    ast::NodeType::List, position,
                    std::vector<ast::TypePtr>(values.begin() + i, values.end())
                );
              }

              // And stop binding as we're at the end
              break;

            } else {
              // Otherwise bind this value to the name
              data[binds[i]] = values[i];
            }
          }
        };

      public:
        template<typename ...T>
        static EnvPtr create(T&& ... all) {
          return std::shared_ptr<Environment>(new Environment(std::forward<T>(all)... ));
        }

        /// Sets a value into this environment
        void set(const std::string name, const ast::TypePtr value);

        /// Finds which environment holds the given key or returns nullptr
        std::shared_ptr<const runtime::Environment> find(const std::string name) const;

        /// Returns the requested TypePtr or returns nullptr
        const ast::TypePtr get(const std::string name) const;
    };
  }
}
