#ifndef TWAI_COMMAND_BUILDER_BASE_HPP
#define TWAI_COMMAND_BUILDER_BASE_HPP

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
    twai_message_t msg = {};
    uint8_t *data;
    std::shared_ptr<TWAICommandFactorySettings> settings;
    std::shared_ptr<CommandLifecycleRegistry> command_lifecycle_registry;

public:
    TWAICommandBuilderBase(std::shared_ptr<TWAICommandFactorySettings> settings, uint8_t command_code) : settings(settings), command_lifecycle_registry(settings->command_lifecycle_registry)
    {
        ESP_LOGI(FUNCTION_NAME, "TWAICommandBuilderBase constructor called");
        set_default_values();
        set_command_code(command_code);
        set_data_length_code(2);
        create_msg_data();
        register_command();
    }

    TWAICommandBuilderBase(std::shared_ptr<TWAICommandFactorySettings> settings, uint8_t command_code, std::vector<uint8_t> payload) : settings(settings), command_lifecycle_registry(settings->command_lifecycle_registry)
    {
        ESP_LOGI(FUNCTION_NAME, "TWAICommandBuilderBase constructor called");
        set_default_values();
        set_command_code(command_code);
        payload = payload;
        set_data_length_code(2 + payload.size());
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

    void register_command()
    {
        // command_lifecycle_registry->register_command(id, command_id);
    }

    void set_command_code(uint8_t code)
    {
        command_code = code;
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
        return enqueue_message();
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

    esp_err_t enqueue_message()
    {

        if (!settings->outQ)
        {
            ESP_LOGE(FUNCTION_NAME, "outQ is NULL");
            return ESP_FAIL;
        }

        // DEBUGGING MSG
        ESP_LOGI(FUNCTION_NAME, "ID: %lu\t length: %u\t code: 0x%02X", msg.identifier, msg.data_length_code, msg.data[0]);
        for (int i = 0; i < msg.data_length_code; i++)
        {
            ESP_LOGI(FUNCTION_NAME, "Data[%d]: %02X", i, msg.data[i]);
        }

        if (xQueueSendToBack(settings->outQ, &msg, 0) != pdTRUE)
        {
            ESP_LOGE(FUNCTION_NAME, " ==> Failed to enqueue message!");
            return ESP_FAIL;
        }
        else
        {
            ESP_LOGI(FUNCTION_NAME, "    ==> Message enqueued successfully!");
            return ESP_OK;
        }
    }
};

#endif // TWAI_COMMAND_BUILDER_BASE_HPP
