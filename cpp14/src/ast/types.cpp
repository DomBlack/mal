#include <vector>
#include <sstream>
#include "types.h"

using namespace mal;

std::ostream &operator<< (std::ostream &os, mal::ast::TypePtr const m) {
  m->toStream(os);
  return os;
}

std::string replaceSubstr(std::string subject, const std::string& search,
                          const std::string& replace) {
  size_t pos = 0;
  while((pos = subject.find(search, pos)) != std::string::npos) {
    subject.replace(pos, search.length(), replace);
    pos += replace.length();
  }
  return subject;
}

void ast::List::toStream(std::ostream &stream) const {
  std::string endToken;

  switch (this->type) {
    case ast::NodeType::List:
      stream << "(";
      endToken = ")";
      break;

    case ast::NodeType::Vector:
      stream << "[";
      endToken = "]";
      break;

    default:
      stream << "<<UNKNOWN TYPE: ";
      endToken = " :UNKNOWN TYPE>>";
      break;
  }

  bool addSpaceBefore = false;

  for (const auto item : this->items) {
    // Add the space before the token if needed
    if (addSpaceBefore) {
      stream << " ";
    }
    addSpaceBefore = true;

    stream << item;
  }

  stream << endToken;
}

void ast::Map::toStream(std::ostream &stream) const {
  stream << "{";

  bool addSpaceBefore = false;

  for (const auto& pair : this->map) {
    if (addSpaceBefore) {
      stream << " ";
    }
    addSpaceBefore = true;

    stream << pair.first << " " << pair.second;
  }

  stream << "}";
}

void ast::Nil::toStream(std::ostream &stream) const {
  stream << "nil";
}

void ast::Boolean::toStream(std::ostream &stream) const {
  if (value) {
    stream << "true";
  } else {
    stream << "false";
  }
}

void ast::Integer::toStream(std::ostream &stream) const {
  stream << value;
}

void ast::String::toStream(std::ostream &stream) const {
  stream << "\"" << replaceSubstr(replaceSubstr(replaceSubstr(value,  "\\", "\\\\"), "\"", "\\\""), "\n", "\\n") << "\"";
}

void ast::Symbol::toStream(std::ostream &stream) const {
  stream << value;
}

void ast::Keyword::toStream(std::ostream &stream) const {
  stream << ":" << value;
}

std::string ast::Type::toString() const {
  std::stringstream stream;

  this->toStream(stream);

  return stream.str();
}
