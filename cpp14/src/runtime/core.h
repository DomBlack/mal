#pragma once

#include <unordered_map>

#include "ast/internal.h"

namespace mal {
  namespace runtime {
    extern const std::unordered_map<std::string, internal::FuncLambda> ns;
  }
}
