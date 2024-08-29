#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <cstring>

inline const char *findFunctionNameStart(const char *prettyFunction)
{
  const char *start = prettyFunction;
  const char *lastSpace = nullptr;
  const char *doubleColon = strstr(prettyFunction, "::");

  while (*start != '\0' && start < doubleColon)
  {
    if (*start == ' ')
    {
      lastSpace = start;
    }
    ++start;
  }

  if (lastSpace)
  {
    start = lastSpace + 1; // Move past the last space
  }
  else
  {
    start = prettyFunction; // No space found, start from the beginning
  }

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
