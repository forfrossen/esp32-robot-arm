#ifndef UTILS_HPP
#define UTILS_HPP

#include "MksEnums.hpp"

#include "../magic_enum/include/magic_enum/magic_enum.hpp"
#include "esp_log.h"
#include <algorithm>
#include <cmath>
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

// else
// {
//     ESP_LOGI(TAG, "Condition met: %s", #cond);
// }
#define CHECK_THAT(cond)                                                    \
    do                                                                      \
    {                                                                       \
        if (!(cond))                                                        \
        {                                                                   \
            ESP_LOGE(FUNCTION_NAME, "Failed to meet condition: %s", #cond); \
            return ESP_FAIL;                                                \
        }                                                                   \
    } while (0)

#define RETURN_IF_NOT(cond)                                     \
    if (!(cond))                                                \
    {                                                           \
        ESP_LOGE(FUNCTION_NAME, "Condition failed: %s", #cond); \
        return;                                                 \
    }

#define RETURN_BOOL(cond)                                        \
    if ((cond))                                                  \
    {                                                            \
        ESP_LOGI(FUNCTION_NAME, "Condition met: %s", #cond);     \
        return true;                                             \
    }                                                            \
    else                                                         \
    {                                                            \
        ESP_LOGE(FUNCTION_NAME, "Condition not met: %s", #cond); \
        return false;                                            \
    }

#define RETURN_VOID_IF(cond)                                 \
    if ((cond))                                              \
    {                                                        \
        ESP_LOGE(FUNCTION_NAME, "Condition met: %s", #cond); \
        return;                                              \
    }

#define RETURN_NPTR_IF(cond)                                 \
    if ((cond))                                              \
    {                                                        \
        ESP_LOGE(FUNCTION_NAME, "Condition met: %s", #cond); \
        return nullptr;                                      \
    }

#define CONT_IF_CHECK_FAILS(cond)                               \
    if (!(cond))                                                \
    {                                                           \
        ESP_LOGE(FUNCTION_NAME, "Condition failed: %s", #cond); \
        continue;                                               \
    }

// #define GET_CMD(msg) replace_underscores(magic_enum::enum_name(static_cast<CommandIds>((msg).data[0])).data()).c_str()
#define GET_MSGCMD(msg) replace_underscores(magic_enum::enum_name(static_cast<CommandIds>(*reinterpret_cast<uint8_t *>(msg->data[0]))).data()).c_str()
#define GET_CMDPTR(cmd) replace_underscores(magic_enum::enum_name(static_cast<CommandIds>(*reinterpret_cast<uint8_t *>(cmd))).data()).c_str()
#define GET_CMD(cmd) magic_enum::enum_name(static_cast<CommandIds>(cmd)).data()

inline std::string replace_underscores(const std::string &str)
{
    std::string result = str;
    std::replace(result.begin(), result.end(), '_', ' ');
    return result;
}

#define SEND_COMMAND_BY_ID(mutex, command_factory, command_id, context, ret)                    \
    do                                                                                          \
    {                                                                                           \
        auto cmd = command_factory->create_command(command_id);                                 \
        ret = cmd->execute();                                                                   \
        if (ret != ESP_OK)                                                                      \
        {                                                                                       \
            ESP_LOGE("SEND_COMMAND_BY_ID", "Error executing command: %s", GET_CMD(command_id)); \
            context->transition_ready_state(MotorContext::ReadyState::MOTOR_ERROR);             \
        }                                                                                       \
        delete cmd;                                                                             \
    } while (0)

#define SEND_COMMAND_BY_ID_WITH_PAYLOAD(mutex, cmd, context, ret)                     \
    do                                                                                \
    {                                                                                 \
        ret = cmd->execute();                                                         \
        xSemaphoreGive(mutex);                                                        \
        if (ret != ESP_OK)                                                            \
        {                                                                             \
            ESP_LOGE("SEND_COMMAND_BY_ID_WITH_PAYLOAD", "Error executing command: "); \
            context->transition_ready_state(MotorContext::ReadyState::MOTOR_ERROR);   \
        }                                                                             \
        delete cmd;                                                                   \
    } while (0)

// Function to get the enum name from twai_message_t.data[0]
inline const char *get_command_name(const twai_message_t *msg)
{
    CommandIds command = static_cast<CommandIds>(msg->data[0]);
    auto enum_name = magic_enum::enum_name(command);

    ESP_LOGI(FUNCTION_NAME, "Command ID: 0x%02X ", msg->data[0]);

    if (!enum_name.empty())
    {
        ESP_LOGI(FUNCTION_NAME, "Command name: %s", enum_name.data());
        return enum_name.data();
    }
    else
    {
        ESP_LOGI(FUNCTION_NAME, "Unknown Command ID");
        return "Unknown Command ID";
    }
}

inline void log_twai_message(const twai_message_t *msg, std::optional<bool> is_received = false)
{
    ESP_LOGI(FUNCTION_NAME, "=================================================================================================");
    CommandIds command_id = static_cast<CommandIds>(msg->data[0]);
    const char *command_name = get_command_name(msg);
    ESP_LOGI(FUNCTION_NAME, "MESSAGE %s", is_received.value() == true ? "RECEIVED <<=====" : "TO BE SENT =====>>");
    ESP_LOGI(FUNCTION_NAME, "ID: 0x%02lu \t length: %d\t command_id: 0x%02X \t command_name: %s",
             msg->identifier, msg->data_length_code, command_id, command_name);
    ESP_LOGI(FUNCTION_NAME, "Extended ID: %s", msg->extd ? "True" : "False");
    ESP_LOGI(FUNCTION_NAME, "RTR: %s", msg->rtr ? "True" : "False");
    for (int i = 0; i < msg->data_length_code - 1; i++)
    {
        ESP_LOGI(FUNCTION_NAME, "  Data[%d]: \t 0x%02X \t %d ", i, msg->data[i], msg->data[i]);
    }
    ESP_LOGI(FUNCTION_NAME, "  Data CRC: \t 0x%02X \t %d ", msg->data[msg->data_length_code - 1], msg->data[msg->data_length_code - 1]);
    ESP_LOGI(FUNCTION_NAME, "=================================================================================================");
}

#endif // UTILS_HPP
