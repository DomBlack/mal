#include "core.h"

#include <string>
#include <iostream>
#include <fstream>
#include <streambuf>
#include <sstream>

#include "helpers.h"
#include "ast/atom.h"

using namespace mal;

#define NS_FUNC(name, code) { name, [](auto callsite, auto funcName, internal::FuncArgs arguments) -> EvalResult code }

#define NUMERIC_OP(op) NS_FUNC(#op, { ARG_COUNT(2); ARG_CAST(lhs, 1, Integer); ARG_CAST(rhs, 2, Integer); RTN_NODE(Integer, lhs->value op rhs->value); })
#define NUMERIC_COMPARE_OP(op) NS_FUNC(#op, { ARG_COUNT(2); ARG_CAST(lhs, 1, Integer); ARG_CAST(rhs, 2, Integer); RTN_NODE(Boolean, lhs->value op rhs->value); })

bool compare(const ast::TypePtr a, const ast::TypePtr b);

/// Compares two primitive values
///
/// \tparam T The primative type
/// \param a  The first node
/// \param b  The second node
///
/// \return true if both nodes hold the same value
template<class T>
bool compare_primitive(const ast::TypePtr a, const ast::TypePtr b) {
  return std::static_pointer_cast<T>(a)->value == std::static_pointer_cast<T>(b)->value;
}

bool compare_lists(const ast::TypePtr a, const ast::TypePtr b) {
  const auto aList = std::static_pointer_cast<ast::List>(a)->items;
  const auto bList = std::static_pointer_cast<ast::List>(b)->items;

  // Check the lists are the same length
  if (aList.size() != bList.size()) {
    return false;
  }

  // Check each item is the same
  for (std::size_t i = 0; i < aList.size(); i++) {
    if (!compare(aList[i], bList[i])) {
      return false;
    }
  }

  return true;
}

bool compare_maps(const ast::TypePtr a, const ast::TypePtr b) {
  const auto aMap = std::static_pointer_cast<ast::Map>(a)->map;
  const auto bMap = std::static_pointer_cast<ast::Map>(b)->map;

  // Check the maps are the same length
  if (aMap.size() != bMap.size()) {
    return false;
  }

  // Check the map contents match
  for (const auto aPair : aMap) {
    if (bMap.count(aPair.first) == 0 || !compare(aPair.second, bMap.at(aPair.first))) {
      return false;
    }
  }

  return true;
}

bool isList(const ast::TypePtr a) {
  return a->type == ast::NodeType::List || a->type == ast::NodeType::Vector;
}

/// Compares two AST nodes
///
/// \param a The first node
/// \param b The second node
///
/// \return true if both nodes hold the same values and types
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"
bool compare(const ast::TypePtr a, const ast::TypePtr b) {

  // Different types
  if (
      (a->type != b->type)
      &&
      !(isList(a) && isList(b)) // if a/b are both list types, then we actually want the switch below
  ) {
    return false;
  }

  switch (a->type) {
    case ast::NodeType::Nil: // Nil === Nil
    case ast::NodeType::Comment: // comments should never get here
      return true;

    case ast::NodeType::List:
    case ast::NodeType::Vector:
      return compare_lists(a, b);

    case ast::NodeType::Map:
      return compare_maps(a, b);

    case ast::NodeType::Boolean:
      return compare_primitive<ast::Boolean>(a, b);

    case ast::NodeType::Integer:
      return compare_primitive<ast::Integer>(a, b);

    case ast::NodeType::String:
      return compare_primitive<ast::String>(a, b);

    case ast::NodeType::Symbol:
      return compare_primitive<ast::Symbol>(a, b);

    case ast::NodeType::Keyword:
      return compare_primitive<ast::Keyword>(a, b);

    case ast::NodeType::InternalFunc:
    case ast::NodeType::UserFunc:
    case ast::NodeType::Atom:
      return a == b;
  }

  std::cerr << "Unknown NodeType in comparision at " << a->line << ":" << a->position <<  " - " << a << std::endl;
  return false;
}
#pragma clang diagnostic pop

std::string pr_str(const std::vector<ast::TypePtr> items, const bool printReadably, const std::string seperator) {
  std::stringstream stream;

  // Set the printable flag on this stream
  stream.iword(mal::printStringNodesRaw) = printReadably ? 0 : 1;

  bool first = true;

  for (auto node : items) {
    if (first) {
      first = false;
    } else {
      stream << seperator;
    }

    stream << node;
  }

  return stream.str();
}

const std::unordered_map<std::string, internal::FuncLambda> runtime::ns = {
    NS_FUNC(
        "list",
        {
          RTN_VALUE(std::make_shared<ast::List>(ast::NodeType::List, callsite->toDummyToken(), arguments));
        }
    ),
    NS_FUNC(
        "list?",
        {
          ARG_COUNT(1);
          RTN_NODE(Boolean, arguments.front()->type == ast::NodeType::List);
        }
    ),
    NS_FUNC(
        "empty?",
        {
          ARG_COUNT(1);

          if (arguments.front()->type == ast::NodeType::Nil) {
            RTN_NODE(Boolean, true);
          }

          ARG_CAST_TO_LIST(list, 1);
          RTN_NODE(Boolean, list->items.empty());
        }
    ),
    NS_FUNC(
        "count",
        {
          ARG_COUNT(1);

          if (arguments.front()->type == ast::NodeType::Nil) {
            RTN_NODE(Integer, 0);
          }

          ARG_CAST_TO_LIST(list, 1);
          RTN_NODE(Integer, list->items.size());
        }
    ),
    NS_FUNC(
      "=",
      {
        ARG_COUNT(2);
        RTN_NODE(Boolean, compare(arguments.front(), arguments.back()));
      }
    ),
    NUMERIC_COMPARE_OP(<),
    NUMERIC_COMPARE_OP(<=),
    NUMERIC_COMPARE_OP(>),
    NUMERIC_COMPARE_OP(>=),
    NUMERIC_OP(+),
    NUMERIC_OP(-),
    NUMERIC_OP(*),
    NUMERIC_OP(/),

    NS_FUNC("pr-str",  { RTN_NODE(String, pr_str(arguments, true, " ")); }),
    NS_FUNC("str",     { RTN_NODE(String, pr_str(arguments, false, "")); }),
    NS_FUNC("prn",     { std::cout << pr_str(arguments, true, " ") << std::endl; RTN_NIL(); }),
    NS_FUNC("println", { std::cout << pr_str(arguments, false, " ") << std::endl; RTN_NIL(); }),
    NS_FUNC(
        "read-string",
        {
          ARG_COUNT(1);
          ARG_CAST(input, 1, String);

          return readString(input->value);
        }
    ),
    NS_FUNC(
      "slurp",
      {
        ARG_COUNT(1);
        ARG_CAST(fileName, 1, String);

        std::ifstream file(fileName->value.c_str());
        if (!file.is_open()) {
          RTN_ERROR("Unable to open \"" + fileName->value + "\"");
        }

        std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        RTN_NODE(String, str);
      }
    ),


    NS_FUNC(
      "atom",
      {
        ARG_COUNT(1);
        RTN_NODE(Atom, arguments.front());
      }
    ),

    NS_FUNC(
      "atom?",
      {
        ARG_COUNT(1);
        RTN_NODE(Boolean, arguments.front()->type == ast::NodeType::Atom);
      }
    ),

    NS_FUNC(
        "deref",
        {
          ARG_COUNT(1);
          ARG_CAST(atom, 1, Atom);
          RTN_VALUE(atom->value);
        }
    ),

    NS_FUNC(
        "reset!",
        {
          ARG_COUNT(2);
          ARG_CAST(atom, 1, Atom);

          // Replace the value
          atom->value = arguments[1];

          RTN_VALUE(arguments[1]);
        }
    ),

    NS_FUNC(
      "cons",
      {
        ARG_COUNT(2);
        ARG_CAST_TO_LIST(list, 2);

        auto copy = list->copyOfItems();

        copy.insert(copy.begin(), arguments[0]);

        RTN_VALUE(
            std::make_shared<ast::List>(ast::NodeType::List, callsite->toDummyToken(), copy)
        );
      }
    ),

    NS_FUNC(
      "concat",
      {
        std::vector<ast::TypePtr> newList;

        for (const auto arg : arguments) {
          CAST_AS_LIST(list, arg);

          for (const auto item : list->items) {
            newList.push_back(item);
          }
        }

        newList.shrink_to_fit();

        RTN_VALUE(
            std::make_shared<ast::List>(ast::NodeType::List, callsite->toDummyToken(), newList)
        );
      }
    ),

    NS_FUNC(
      "nth",
      {
        ARG_COUNT(2);

        ARG_CAST_TO_LIST(list, 1);
        ARG_CAST(index, 2, Integer);

        if (index->value < 0 || index->value >= list->items.size()) {
          RTN_ERROR("Index out of bounds"); // FIXME change to "exception"
        }

        RTN_VALUE(list->items[index->value]);
      }
    ),

    NS_FUNC(
      "first",
      {
        ARG_COUNT(1);

        if (arguments[0]->type == ast::NodeType::Nil) {
          RTN_NIL();
        }

        ARG_CAST_TO_LIST(list, 1);

        if (list->items.empty()) {
          RTN_NIL();
        } else {
          RTN_VALUE(list->items.front());
        }
      }
    ),

    NS_FUNC(
      "rest",
      {
        ARG_COUNT(1);

        if (arguments[0]->type == ast::NodeType::Nil) {
          std::vector<ast::TypePtr> newList = {};
          RTN_VALUE(std::make_shared<ast::List>(ast::NodeType::List, callsite->toDummyToken(), newList));
        }

        ARG_CAST_TO_LIST(list, 1);

        if (list->items.empty()) {
          std::vector<ast::TypePtr> newList = {};
          RTN_VALUE(std::make_shared<ast::List>(ast::NodeType::List, callsite->toDummyToken(), newList));
        } else {
          const auto newList = std::vector<ast::TypePtr>(list->items.begin() + 1, list->items.end());

          RTN_VALUE(std::make_shared<ast::List>(ast::NodeType::List, callsite->toDummyToken(), newList));
        }
      }
    )
};
