#include "internal.h"

#include <ostream>

#include "ast/types.h"
#include "runtime/helpers.h"

using namespace mal;

void internal::InternalFunc::toStream(std::ostream &stream) const {
  stream << this->name;
}
