#ifndef UTILS_HPP
#define UTILS_HPP

#include "../magic_enum/include/magic_enum/magic_enum.hpp"
#include "MksEnums.hpp"
#include <algorithm>
#include <cstring>
#include <hal/twai_types.h>
#include <string>

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

// #define GET_CMD(msg) replace_underscores(magic_enum::enum_name(static_cast<CommandIds>((msg).data[0])).data()).c_str()
#define GET_MSGCMD(msg) replace_underscores(magic_enum::enum_name(static_cast<CommandIds>(*reinterpret_cast<uint8_t *>(msg->data))).data()).c_str()
#define GET_CMDPTR(cmd) replace_underscores(magic_enum::enum_name(static_cast<CommandIds>(*reinterpret_cast<uint8_t *>(cmd))).data()).c_str()
#define GET_CMD(cmd) magic_enum::enum_name(static_cast<CommandIds>(cmd)).data()

inline std::string replace_underscores(const std::string &str)
{
    std::string result = str;
    std::replace(result.begin(), result.end(), '_', ' ');
    return result;
}

#define SEND_COMMAND_BY_ID(mutex, command_factory, command_id, context, ret)        \
    do                                                                              \
    {                                                                               \
        if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE)                         \
        {                                                                           \
            auto cmd = command_factory->generate_new_generic_builder(command_id);   \
            ret = cmd->build_and_send();                                            \
            xSemaphoreGive(mutex);                                                  \
        }                                                                           \
        else                                                                        \
        {                                                                           \
            ESP_LOGE(FUNCTION_NAME, "Failed to take mutex");                        \
            context->transition_ready_state(MotorContext::ReadyState::MOTOR_ERROR); \
            ret = ESP_FAIL;                                                         \
        }                                                                           \
    } while (0)

#define SEND_COMMAND_BY_ID_WITH_PAYLOAD(mutex, command_factory, cmd, context, ret)  \
    do                                                                              \
    {                                                                               \
        esp_err_t err = ESP_FAIL;                                                   \
        if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE)                         \
        {                                                                           \
            ret = cmd->build_and_send();                                            \
            xSemaphoreGive(mutex);                                                  \
        }                                                                           \
        else                                                                        \
        {                                                                           \
            ESP_LOGE(FUNCTION_NAME, "Failed to take mutex");                        \
            context->transition_ready_state(MotorContext::ReadyState::MOTOR_ERROR); \
            ret = ESP_FAIL;                                                         \
        }                                                                           \
                                                                                    \
    } while (0)
#endif // UTILS_HPP
