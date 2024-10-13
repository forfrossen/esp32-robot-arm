#ifndef UTILS_HPP
#define UTILS_HPP

#include "../magic_enum/include/magic_enum/magic_enum.hpp"
#include "MksEnums.hpp"
#include "esp_log.h"
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

// else
// {
//     ESP_LOGI(TAG, "Condition met: %s", #cond);
// }
#define CHECK_THAT(cond, TAG)                                     \
    do                                                            \
    {                                                             \
        if (!(cond))                                              \
        {                                                         \
            ESP_LOGE(TAG, "Failed to meet condition: %s", #cond); \
            return ESP_FAIL;                                      \
        }                                                         \
    } while (0)

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

#define SEND_COMMAND_BY_ID(mutex, command_factory, command_id, context, ret)        \
    do                                                                              \
    {                                                                               \
        auto cmd = command_factory->generate_new_generic_command(command_id);       \
        ret = cmd->execute();                                                       \
        if (ret != ESP_OK)                                                          \
        {                                                                           \
            context->transition_ready_state(MotorContext::ReadyState::MOTOR_ERROR); \
        }                                                                           \
    } while (0)

#define SEND_COMMAND_BY_ID_WITH_PAYLOAD(mutex, command_factory, cmd, context, ret)  \
    do                                                                              \
    {                                                                               \
        esp_err_t err = ESP_FAIL;                                                   \
        if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE)                         \
        {                                                                           \
            ret = cmd->execute();                                                   \
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

// Function to get the enum name from twai_message_t.data[0]
static const char *get_command_name(const twai_message_t &msg)
{
    // Get the value of the first byte (data[0])
    uint8_t command_value = msg.data[0];

    // Try to get the enum name from the value using magic_enum
    auto enum_name = magic_enum::enum_name(static_cast<CommandIds>(command_value));

    // If the enum name is not found, return a default message
    if (enum_name.empty())
    {
        return "Unknown Command ID";
    }

    // Return the enum name as a C-style string
    return enum_name.data();
}

static void log_twai_message(const twai_message_t *msg)
{
    ESP_LOGI("TWAI_LOG", "=================================================================================================");
    const char *command_name = get_command_name(*msg);
    ESP_LOGI("TWAI_LOG", "ID: 0x%02lu \t length: %d / %02u \t code: 0x%02X \t commandName: %s", msg->identifier, msg->data_length_code, msg->data_length_code, msg->data[0], command_name);
    ESP_LOGI("TWAI_LOG", "Extended ID: %s", msg->extd ? "True" : "False");
    // Log the RTR (Remote Transmission Request) flag
    ESP_LOGI("TWAI_LOG", "RTR: %s", msg->rtr ? "True" : "False");
    for (int i = 0; i < msg->data_length_code - 1; i++)
    {
        ESP_LOGI(FUNCTION_NAME, "  Data[%d]: \t 0x%02X \t %d ", i, msg->data[i], msg->data[i]);
    }
    ESP_LOGI(FUNCTION_NAME, "  Data CRC: \t 0x%02X \t %d ", msg->data[msg->data_length_code - 1], msg->data[msg->data_length_code - 1]);

    ESP_LOGI("TWAI_LOG", "=================================================================================================");
}

#endif // UTILS_HPP
