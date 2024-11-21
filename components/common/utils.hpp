#ifndef UTILS_HPP
#define UTILS_HPP

#include "MksEnums.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <hal/twai_types.h>
#include <iostream>
#include <magic_enum.hpp>
#include <sstream>
#include <string>
#include <vector>

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

// Logging macros with function name
#define LOGE(format, ...) ESP_LOGE(tag, "%s: " format, FUNCTION_NAME, ##__VA_ARGS__)
#define LOGW(format, ...) ESP_LOGW(tag, "%s: " format, FUNCTION_NAME, ##__VA_ARGS__)
#define LOGI(format, ...) ESP_LOGD(tag, "%s: " format, FUNCTION_NAME, ##__VA_ARGS__)
#define LOGD(format, ...) ESP_LOGD(tag, "%s: " format, FUNCTION_NAME, ##__VA_ARGS__)
#define LOGV(format, ...) ESP_LOGD(tag, "%s: " format, FUNCTION_NAME, ##__VA_ARGS__)

// else
// {
//     ESP_LOGD(TAG, "Condition met: %s", #cond);
// }

#define GOTO_ON_ERROR(goto_tag, format, ...)                                                   \
    do                                                                                         \
    {                                                                                          \
        if (unlikely(ret != ESP_OK))                                                           \
        {                                                                                      \
            ESP_LOGE(FUNCTION_NAME, "%s(%d): " format, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            goto goto_tag;                                                                     \
        }                                                                                      \
    } while (0)

#define CHECK_THAT(cond)                                                    \
    do                                                                      \
    {                                                                       \
        if (!(cond))                                                        \
        {                                                                   \
            ESP_LOGE(FUNCTION_NAME, "Failed to meet condition: %s", #cond); \
            return ESP_FAIL;                                                \
        }                                                                   \
    } while (0)

#define ERRCHECK_OR_DELETE_COMMAND(ret)                                 \
    do                                                                  \
    {                                                                   \
        if (ret != ESP_OK)                                              \
        {                                                               \
            ESP_LOGE(FUNCTION_NAME, "Error: %s", esp_err_to_name(ret)); \
            delete command;                                             \
            return ret;                                                 \
        }                                                               \
    } while (0)

#define ERRCHECK_OR_DELETE_THIS(ret)                                    \
    do                                                                  \
    {                                                                   \
        if (ret != ESP_OK)                                              \
        {                                                               \
            ESP_LOGE(FUNCTION_NAME, "Error: %s", esp_err_to_name(ret)); \
            delete this;                                                \
            return ret;                                                 \
        }                                                               \
    } while (0)

#define CHECK_THAT_AND_LOG(x, y, z) \
    if (!(x))                       \
    {                               \
        ESP_LOGE(FUNCTION_NAME, y); \
        z;                          \
    }

#define ERROR_CHECK_THAT_AND_LOG(x, y, z) \
    if (!(x))                             \
    {                                     \
        ESP_LOGE(FUNCTION_NAME, y);       \
        z;                                \
    }

#define RETURN_IF_NOT(cond)                                     \
    if (!(cond))                                                \
    {                                                           \
        ESP_LOGE(FUNCTION_NAME, "Condition failed: %s", #cond); \
        return;                                                 \
    }

#define RETURN_BOOL(cond)                                        \
    if ((cond))                                                  \
    {                                                            \
        ESP_LOGD(FUNCTION_NAME, "Condition met: %s", #cond);     \
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

#define RETURN_NULLPTR_ON_ESPERROR(ret)                             \
    if (ret != ESP_OK)                                              \
    {                                                               \
        ESP_LOGE(FUNCTION_NAME, "Error: %s", esp_err_to_name(ret)); \
        return nullptr;                                             \
    }

#define CONT_IF_CHECK_FAILS(cond)                               \
    if (!(cond))                                                \
    {                                                           \
        ESP_LOGE(FUNCTION_NAME, "Condition failed: %s", #cond); \
        continue;                                               \
    }

// #define GET_CMD(msg) replace_underscores(magic_enum::enum_name(static_cast<motor_command_t>((msg).data[0])).data()).c_str()
#define GET_MSGCMD(msg) replace_underscores(magic_enum::enum_name(static_cast<motor_command_id_t>(*reinterpret_cast<uint8_t *>(msg->data[0]))).data()).c_str()
#define GET_CMDPTR(cmd) replace_underscores(magic_enum::enum_name(static_cast<motor_command_id_t>(*reinterpret_cast<uint8_t *>(cmd))).data()).c_str()
#define GET_CMD(cmd) magic_enum::enum_name(static_cast<motor_command_id_t>(cmd)).data()

inline std::string replace_underscores(const std::string &str)
{
    std::string result = str;
    std::replace(result.begin(), result.end(), '_', ' ');
    return result;
}

#define SEND_COMMAND_BY_ID(command_factory, command_id, context, ret)                           \
    do                                                                                          \
    {                                                                                           \
        auto cmd = command_factory->create_command(command_id);                                 \
        CHECK_THAT(cmd != nullptr);                                                             \
        ret = cmd->execute();                                                                   \
        if (ret != ESP_OK)                                                                      \
        {                                                                                       \
            ESP_LOGE("SEND_COMMAND_BY_ID", "Error executing command: %s", GET_CMD(command_id)); \
            context->transition_ready_state(MotorContext::ReadyState::MOTOR_ERROR);             \
        }                                                                                       \
        delete cmd;                                                                             \
    } while (0)

#define SEND_COMMAND_BY_ID_WITH_PAYLOAD(cmd, context, ret)                            \
    do                                                                                \
    {                                                                                 \
        CHECK_THAT(cmd != nullptr);                                                   \
        ret = cmd->execute();                                                         \
        CHECK_THAT(ret == ESP_OK);                                                    \
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
    motor_command_id_t command = static_cast<motor_command_id_t>(msg->data[0]);
    auto enum_name = magic_enum::enum_name(command);

    ESP_LOGD(FUNCTION_NAME, "Command ID: 0x%02X ", msg->data[0]);

    if (!enum_name.empty())
    {
        ESP_LOGD(FUNCTION_NAME, "Command name: %s", enum_name.data());
        return enum_name.data();
    }
    else
    {
        ESP_LOGD(FUNCTION_NAME, "Unknown Command ID");
        return "Unknown Command ID";
    }
}

inline void log_twai_message(const twai_message_t *msg, std::optional<bool> is_received = false)
{
    ESP_LOGD(FUNCTION_NAME, "=================================================================================================");
    motor_command_id_t command_id = static_cast<motor_command_id_t>(msg->data[0]);
    const char *command_name = get_command_name(msg);
    ESP_LOGD(FUNCTION_NAME, "MESSAGE %s", is_received.value() == true ? "RECEIVED <<=====" : "TO BE SENT =====>>");
    ESP_LOGD(FUNCTION_NAME, "ID: 0x%02lu \t length: %d\t command_id: 0x%02X \t command_name: %s",
             msg->identifier, msg->data_length_code, command_id, command_name);
    ESP_LOGD(FUNCTION_NAME, "Extended ID: %s", msg->extd ? "True" : "False");
    ESP_LOGD(FUNCTION_NAME, "RTR: %s", msg->rtr ? "True" : "False");
    for (int i = 0; i < msg->data_length_code - 1; i++)
    {
        ESP_LOGD(FUNCTION_NAME, "  Data[%d]: \t 0x%02X \t %d ", i, msg->data[i], msg->data[i]);
    }
    ESP_LOGD(FUNCTION_NAME, "  Data CRC: \t 0x%02X \t %d ", msg->data[msg->data_length_code - 1], msg->data[msg->data_length_code - 1]);
    ESP_LOGD(FUNCTION_NAME, "=================================================================================================");
}

inline esp_err_t get_task_state_without_panic(TaskHandle_t task_handle, eTaskState *task_state)
{
    const char *task_name = pcTaskGetName(task_handle);
    if (task_handle == nullptr)
    {
        ESP_LOGW(FUNCTION_NAME, "Task handle for %s is null!", task_name);
        *task_state = eDeleted;
    }
    else
    {
        *task_state = eTaskGetState(task_handle);
        if (*task_state == eDeleted || *task_state == eInvalid)
        {
            ESP_LOGW(FUNCTION_NAME, "Task handle for %s is invalid or deleted!", task_name);
        }
    }

    ESP_LOGD(FUNCTION_NAME, "Task handle for %s is in state: %s", task_name, magic_enum::enum_name(*task_state).data());

    return ESP_OK;
}

#define IS_STATUS_IN_DATA1(command_id) (std::find(g_no_status_in_data1.begin(), g_no_status_in_data1.end(), command_id) == g_no_status_in_data1.end())

inline esp_err_t split_ws_msg(const std::string &str, std::pair<std::string, std::string> &msg)
{
    size_t delimiter_pos = str.find(':');
    if (delimiter_pos == std::string::npos)
    {
        ESP_LOGI(FUNCTION_NAME, "Delimiter not found in message: %s", str.c_str());
        msg.first = str;
    }
    else
    {
        msg.first = str.substr(0, delimiter_pos);
        msg.second = str.substr(delimiter_pos + 1);
    }

    return ESP_OK;
}

inline bool wait_for_bits(EventGroupHandle_t event_group, ...)
{
    va_list args;
    va_start(args, event_group);

    EventBits_t bits_to_wait_for = 0;
    EventBits_t bit;
    while ((bit = va_arg(args, EventBits_t)) != 0)
    {
        bits_to_wait_for |= bit;
    }
    va_end(args);

    EventBits_t bits = xEventGroupWaitBits(event_group, bits_to_wait_for, pdFALSE, pdTRUE, pdMS_TO_TICKS(500));

    if ((bits & bits_to_wait_for) == bits_to_wait_for)
    {
        return true;
    }
    else
    {
        return false;
    }
}

#define WAIT_FOR_BITS(event_group, ...) \
    (wait_for_bits(event_group, __VA_ARGS__, 0))

#define WAIT_FOR_CONDITIONS(...)                  \
    for (;;)                                      \
    {                                             \
        if (!(__VA_ARGS__))                       \
        {                                         \
            vTaskDelay(500 / portTICK_PERIOD_MS); \
            break;                                \
        }                                         \
    }
#endif // UTILS_HPP
