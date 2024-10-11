#ifndef TWAI_COMMAND_BUILDER_BASE_HPP
#define TWAI_COMMAND_BUILDER_BASE_HPP

#include "../common/utils.hpp"
#include "Events.hpp"
#include "TypeDefs.hpp"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <driver/twai.h>
#include <vector>

class CommandLifecycleRegistry;
template <typename T>
class TWAICommandBuilderBase
{
protected:
    uint8_t command_code;
    twai_message_t msg = {};
    uint8_t *data;
    std::shared_ptr<TWAICommandFactorySettings> settings;
    std::shared_ptr<CommandLifecycleRegistry> command_lifecycle_registry;

public:
    TWAICommandBuilderBase(std::shared_ptr<TWAICommandFactorySettings> settings, CommandIds command_code) : settings(settings), command_lifecycle_registry(settings->command_lifecycle_registry)
    {
        ESP_LOGI(FUNCTION_NAME, "TWAICommandBuilderBase constructor called");
        set_default_values();
        set_command_code(command_code);
        auto it = g_command_payload_map.find(command_code);
        if (it == g_command_payload_map.end())
        {
            ESP_LOGE(FUNCTION_NAME, "Command code not found in command_payload_map");
            return;
        }

        set_data_length_code(2 + calculate_payload_size(it->second));
        create_msg_data();
        register_command();
    }

    virtual ~TWAICommandBuilderBase() = default;

    void set_default_values()
    {
        msg.extd = 0;
        msg.rtr = 0;
        msg.ss = 0;
        msg.self = 0;
        msg.dlc_non_comp = 0;
        msg.identifier = settings->id;
    }

    uint8_t calculate_payload_size(const CommandPayloadInfo &payload_info)
    {
        uint8_t total_size = 2;

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
        command_code = static_cast<uint8_t>(code);
    }

    void set_data_length_code(uint8_t length)
    {
        msg.data_length_code = length;
    }

    void create_msg_data()
    {
        data = new uint8_t[msg.data_length_code];
    }

    virtual esp_err_t build_twai_message() = 0;

    esp_err_t build_and_send()
    {
        build_twai_message();
        set_msg_data();
        set_msg_data_crc();
        ESP_ERROR_CHECK(post_event(&msg));
        return ESP_OK;
    }

    void set_msg_data()
    {
        memcpy(&msg.data, data, msg.data_length_code);
    }

    void set_msg_data_crc()
    {
        msg.data[msg.data_length_code - 1] = calculate_crc();
    }

    uint8_t calculate_crc()
    {
        uint8_t crc = msg.identifier;

        for (uint8_t i = 0; i < msg.data_length_code - 1; i++)
        {
            crc += msg.data[i];
        }

        return crc & 0xFF;
    }

    esp_err_t post_event(twai_message_t *msg)
    {
        ESP_LOGI(FUNCTION_NAME, "Posting event to TWAI_EVENTS");
        return esp_event_post_to(settings->system_event_loop, SYSTEM_EVENTS, OUTGOING_MESSAGE_EVENT, msg, sizeof(twai_message_t), portMAX_DELAY);
    }
};

#endif // TWAI_COMMAND_BUILDER_BASE_HPP
