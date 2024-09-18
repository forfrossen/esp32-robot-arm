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

template <typename T>
class TWAICommandBuilderBase
{
protected:
    uint8_t command_code;
    twai_message_t msg = {};
    uint32_t id;
    QueueHandle_t outQ;
    QueueHandle_t inQ;
    uint8_t *data;
    uint8_t command_id;

public:
    TWAICommandBuilderBase(TWAICommandFactorySettings &settings) : id(settings.id), outQ(settings.outQ), inQ(settings.inQ)
    {
        ESP_LOGI(FUNCTION_NAME, "TWAICommandBuilderBase constructor called");

        msg.extd = 0;
        msg.rtr = 0;
        msg.ss = 0;
        msg.dlc_non_comp = 0;
        msg.identifier = id;
    }

    void vTask_handleIncoming(void *pvParameters)
    {
        TWAICommandBuilderBase *instance = static_cast<TWAICommandBuilderBase *>(pvParameters);
        for (;;)
        {
            twai_message_t msg;
            xQueuePeek(instance->inQ, &msg, (TickType_t)0);
            if (msg.identifier == instance->id && msg.data[0] == instance->command_id)
            {
                ESP_LOGI(FUNCTION_NAME, "Resolving message from inQ with ID: 0x%02lX for command: %02X", msg.identifier, msg.data[0]);
                xQueueReceive(instance->inQ, &msg, (TickType_t)10);
            }
        }
    }

    ~TWAICommandBuilderBase()
    {
        delete[] data;
    }

    void set_command_code(uint8_t code)
    {
        command_code = code;
    }

    void set_data_length_code(int length)
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

        return crc;
    }

    esp_err_t enqueue_message()
    {
        command_id = msg.data[0];

        if (!outQ)
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

        if (xQueueSendToBack(outQ, &msg, 0) != pdTRUE)
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
