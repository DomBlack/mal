#include "atom.h"

void mal::ast::Atom::toStream(std::ostream &stream) const {
  stream << "(atom " << value << ")";
}

