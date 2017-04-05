#include "environment.h"
#include "helpers.h"
#include "ast/internal.h"

using namespace mal;

void runtime::Environment::set(const std::string name, const ast::TypePtr value) {
  this->data[name] = value;
}

std::shared_ptr<const runtime::Environment> runtime::Environment::find(const std::string name) const {
  if (this->data.find(name) == this->data.end()) {
    if (this->outer == nullptr) {
      return nullptr;
    } else {
      return this->outer->find(name);
    }
  } else {
    return shared_from_this();
  }
}

const ast::TypePtr runtime::Environment::get(const std::string name) const {
  if (this->data.find(name) == this->data.end()) {
    return nullptr;
  } else {
    return this->data.at(name);
  }
}
