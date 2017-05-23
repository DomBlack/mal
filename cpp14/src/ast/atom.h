#pragma once

#include "types.h"

namespace mal {
  namespace ast {
    class Atom : public Type {
      public:
        TypePtr value;

        Atom(const Token token, TypePtr value): Type(NodeType::Atom, token), value(value) {};

        void toStream(std::ostream &stream) const override;
    };
  }
}
