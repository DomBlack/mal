#include "userFunc.h"

void mal::ast::UserFunc::toStream(std::ostream &stream) const {
  if (this->_isMacro) {
    stream << "#<User Defined Macro>";
  } else {
    stream << "#<User Defined Function>";
  }
}
