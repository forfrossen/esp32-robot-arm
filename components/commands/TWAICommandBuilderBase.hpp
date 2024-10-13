#ifndef TWAI_COMMAND_BUILDER_BASE_HPP
#define TWAI_COMMAND_BUILDER_BASE_HPP

#include "../common/utils.hpp"
#include "Events.hpp"
#include "TypeDefs.hpp"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "utils.hpp"
#include <driver/twai.h>
#include <vector>

class CommandLifecycleRegistry;
template <typename T>
class TWAICommandBuilderBase
{
protected:
    uint8_t command_code;
    std::unique_ptr<twai_message_t> msg;
    uint8_t *data;
    std::shared_ptr<TWAICommandFactorySettings> settings;
    std::shared_ptr<CommandLifecycleRegistry> command_lifecycle_registry;
    SemaphoreHandle_t msg_mutex;

public:
    TWAICommandBuilderBase(std::shared_ptr<TWAICommandFactorySettings> settings, CommandIds cmd_code) : settings(settings), command_lifecycle_registry(settings->command_lifecycle_registry)
    {
        msg_mutex = xSemaphoreCreateMutex();
        if (xSemaphoreTake(msg_mutex, portMAX_DELAY) == pdTRUE)
        {
            assert(settings && "TWAICommandFactorySettings is null");
            assert(command_lifecycle_registry && "CommandLifecycleRegistry is null");
            msg = std::make_unique<twai_message_t>();

            uint8_t command_code_int = static_cast<uint8_t>(cmd_code);
            ESP_LOGI(FUNCTION_NAME, "TWAICommandBuilderBase constructor called for command_code: 0x%02X", command_code_int);

            set_default_values();
            set_command_code(cmd_code);

            auto it = g_command_payload_map.find(cmd_code);

            if (it == g_command_payload_map.end())
            {
                ESP_LOGE(FUNCTION_NAME, "Command code not found in command_payload_map");
                return;
            }

            ESP_LOGI(FUNCTION_NAME, "Setting data length code to: %d", 2 + calculate_payload_size(it->second));
            set_data_length_code(2 + calculate_payload_size(it->second));

            create_msg_data();
            register_command();
            xSemaphoreGive(msg_mutex);
        }
        else
        {
            ESP_LOGE(FUNCTION_NAME, "Failed to take mutex");
        }
    }

    virtual ~TWAICommandBuilderBase() = default;

    void set_default_values()
    {

        ESP_LOGI(FUNCTION_NAME, "Setting default values");
        msg->extd = 0;
        msg->rtr = 0;
        msg->ss = 0;
        msg->self = 0;
        msg->dlc_non_comp = 0;
        msg->identifier = settings->id;
    }

    uint8_t
    calculate_payload_size(const CommandPayloadInfo &payload_info)
    {

        uint8_t total_size = 0;

        for (const auto &type : payload_info.payload_types)
        {
            switch (type)
            {
            case PayloadType::UINT8:
                total_size += sizeof(uint8_t);
                break;
            case PayloadType::UINT16:
                total_size += sizeof(uint16_t);
                break;
            case PayloadType::UINT32:
                total_size += sizeof(uint32_t);
                break;
            case PayloadType::VOID:
                // No size to add for VOID
                break;
            default:
                ESP_LOGE(FUNCTION_NAME, "Unsupported payload type");
            }
        }
        assert(total_size <= 8);
        return total_size;
    }

    void register_command()
    {
        // command_lifecycle_registry->register_command(id, command_id);
    }

    void set_command_code(CommandIds code)
    {
        ESP_LOGI(FUNCTION_NAME, "Setting command code to: 0x%02X", static_cast<uint8_t>(code));
        command_code = static_cast<uint8_t>(code);
        ESP_LOGI(FUNCTION_NAME, "Command code: 0x%02X", command_code);
        memset(&msg, 0, sizeof(msg));
        ESP_LOGI(FUNCTION_NAME, "Command msg has been zeroed out");
    }

    void set_data_length_code(uint8_t length)
    {

        if (msg == nullptr)
        {
            ESP_LOGE(FUNCTION_NAME, "msg pointer is null!");
            return; // Avoid accessing a null pointer
        }

        ESP_LOGI(FUNCTION_NAME, "Setting data length code to: %d", length);
        msg->data_length_code = length;
        ESP_LOGI(FUNCTION_NAME, "Data length code: %d", msg->data_length_code);
    }

    void create_msg_data()
    {
        ESP_LOGI(FUNCTION_NAME, "Creating message data");
        data = new uint8_t[msg->data_length_code];
        memset(data, 0, sizeof(data));
    }

    virtual esp_err_t build_twai_message() = 0;

    esp_err_t build_and_send()
    {
        if (msg == nullptr)
        {
            ESP_LOGE(FUNCTION_NAME, "msg pointer is null!");
            return ESP_FAIL;
        }

        if (xSemaphoreTake(msg_mutex, portMAX_DELAY) == pdTRUE)
        {
            ESP_LOGI(FUNCTION_NAME, "Building and sending message");
            build_twai_message();
            set_msg_data();
            set_msg_data_crc();
            esp_err_t ret = post_event();
            if (ret != ESP_OK)
            {
                ESP_LOGE(FUNCTION_NAME, "Error posting event. Error code: %d", ret);
                return ret;
            }
            xSemaphoreGive(msg_mutex);
            return ESP_OK;
        }
        else
        {
            ESP_LOGE(FUNCTION_NAME, "Failed to take mutex");
            return ESP_FAIL;
        }
    }

    void set_msg_data()
    {
        if (xSemaphoreTake(msg_mutex, portMAX_DELAY) == pdTRUE)
        {
            ESP_LOGI(FUNCTION_NAME, "Setting message data");
            // Ensure msg->data_length_code is within valid bounds
            if (msg->data_length_code > sizeof(msg->data))
            {
                ESP_LOGE(FUNCTION_NAME, "Data length code is out of bounds");
                abort(); // or handle the error appropriately
            }

            memcpy(&msg->data, data, msg->data_length_code);
            xSemaphoreGive(msg_mutex);
        }
        else
        {
            ESP_LOGE(FUNCTION_NAME, "Failed to take mutex");
        }
    }

    void set_msg_data_crc()
    {
        if (xSemaphoreTake(msg_mutex, portMAX_DELAY) == pdTRUE)
        {
            ESP_LOGI(FUNCTION_NAME, "Setting message data CRC");
            // Ensure msg->data_length_code is valid
            if (msg->data_length_code == 0)
            {
                ESP_LOGE(FUNCTION_NAME, "Data length code is zero");
                abort(); // or handle the error appropriately
            }

            msg->data[msg->data_length_code - 1] = calculate_crc();
            xSemaphoreGive(msg_mutex);
        }
        else
        {
            ESP_LOGE(FUNCTION_NAME, "Failed to take mutex");
        }
    }

    uint8_t calculate_crc()
    {

        ESP_LOGI(FUNCTION_NAME, "Calculating CRC");
        uint8_t crc = msg->identifier;

        for (uint8_t i = 0; i < msg->data_length_code - 1; i++)
        {
            crc += msg->data[i];
        }

        return crc & 0xFF;
    }

    esp_err_t post_event()
    {
        if (xSemaphoreTake(msg_mutex, portMAX_DELAY) == pdTRUE)
        {
            ESP_LOGI(FUNCTION_NAME, "Posting event to SYSTEM_EVENTS");
            return esp_event_post_to(
                settings->system_event_loop,
                SYSTEM_EVENTS,
                OUTGOING_MESSAGE_EVENT,
                (void *)msg.get(),
                sizeof(twai_message_t),
                portMAX_DELAY);
            xSemaphoreGive(msg_mutex);
        }
        else
        {
            ESP_LOGE(FUNCTION_NAME, "Failed to take mutex");
            return ESP_FAIL;
        }
    }
};

#endif // TWAI_COMMAND_BUILDER_BASE_HPP
