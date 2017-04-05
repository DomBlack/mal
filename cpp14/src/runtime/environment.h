#pragma once

#include <unordered_map>

#include "ast/types.h"
#include "ast/internal.h"

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
          data["+"] = internal::addFunc;
          data["-"] = internal::subFunc;
          data["*"] = internal::mulFunc;
          data["/"] = internal::divFunc;
        };

        /// Creates an inner environment
        Environment(const EnvPtr outer): outer(outer) {};

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
