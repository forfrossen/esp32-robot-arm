#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>

inline const char *getFunctionName(const std::string &prettyFunction)
{
  static std::string functionName;
  size_t start = prettyFunction.find('(');        // Find the first space and move to the next character
  functionName = prettyFunction.substr(0, start); // Extract the function name
  return functionName.c_str();
}

#define FUNCTION_NAME getFunctionName(__PRETTY_FUNCTION__)

#endif // UTILS_HPP