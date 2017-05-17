#pragma once

#include <vector>
#include <memory>
#include <map>

#include "parser/tokenizer.h"

#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x) x
#endif

#define PRIMITIVE_TYPE(name, T) class name : public Type { \
  public: \
    const T value; \
\
    name(const Token token, const T value): Type(NodeType::name, token), value(value) {}; \
\
    void toStream(std::ostream &stream) const override; \
}

namespace mal {
  /// iword variable for streams to print nodes raw
  extern const int printStringNodesRaw;

  namespace ast {
    enum class NodeType {
      InternalFunc, Comment, Nil, Boolean, Integer, String, Symbol, Keyword, List, Vector, Map
    };

    class Type {
      public:
        const NodeType type;
        const unsigned int line;
        const unsigned int position;

        Type(const NodeType type, const Token token): type(type), line(token.line), position(token.position) {};
        virtual ~Type() {};

        virtual void toStream(std::ostream& stream) const = 0;

        std::string toString() const;
        Token toDummyToken() const;
    };

    typedef std::shared_ptr<ast::Type> TypePtr;

    class List : public Type {
      public:
        const std::vector<TypePtr> items;

        List(const NodeType type, const Token token, const std::vector<TypePtr> items): Type(type, token), items(items) {};

        void toStream(std::ostream &stream) const override;
    };

    class Map : public Type {
      public:
        const std::map<std::string, TypePtr> map;

        Map(const Token token, std::map<std::string, TypePtr> map): Type(ast::NodeType::Map, token), map(map) {};

        void toStream(std::ostream &stream) const override;
    };

    class Comment : public Type {
      public:
        Comment(const Token token): Type(NodeType::Comment, token) {};

        void toStream(std::ostream &UNUSED(stream)) const override {};
    };

    class Nil : public Type {
      public:
        Nil(const Token token): Type(NodeType::Nil, token) {};

        void toStream(std::ostream &stream) const override;
    };

    PRIMITIVE_TYPE(Boolean, bool);
    PRIMITIVE_TYPE(Integer, int32_t);
    PRIMITIVE_TYPE(String,  std::string);
    PRIMITIVE_TYPE(Symbol,  std::string);
    PRIMITIVE_TYPE(Keyword, std::string);
  }
}

std::ostream &operator<< (std::ostream &os, mal::ast::TypePtr const m);
