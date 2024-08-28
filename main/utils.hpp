#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>

inline const char *findFunctionNameStart(const char *prettyFunction)
{
  const char *start = prettyFunction;
  while (*start != ' ' && *start != '(')
    ++start; // Skip return type or move to the opening parenthesis
  if (*start == ' ')
    ++start; // Move past the space if it is a space
  while (*start == ' ')
    ++start; // Skip any additional spaces
  return start;
}

inline const char *findFunctionNameEnd(const char *start)
{
  const char *end = start;
  while (*end != '(')
    ++end; // Find the opening parenthesis
  return end;
}

inline const char *getFunctionName(const char *prettyFunction)
{
  const char *start = findFunctionNameStart(prettyFunction);
  const char *end = findFunctionNameEnd(start);

  static thread_local char functionName[256];
  size_t length = end - start;
  if (length >= sizeof(functionName))
  {
    length = sizeof(functionName) - 1;
  }
  strncpy(functionName, start, length);
  functionName[length] = '\0';
  return functionName;
}

#define FUNCTION_NAME getFunctionName(__PRETTY_FUNCTION__)

#endif // UTILS_HPP