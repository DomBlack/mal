#include <algorithm>

#include "helpers.h"

bool isIntegerString(const std::string &str)
{
  return std::all_of(str.begin(), str.end(), ::isdigit);
}
