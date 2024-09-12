#ifndef TWAI_COMMAND_BUILDER_BASE_HPP
#define TWAI_COMMAND_BUILDER_BASE_HPP

#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "utils.hpp"
#include <driver/twai.h>

class TWAICommandBuilderBase
{
protected:
    twai_message_t msg = {};
    QueueHandle_t outQ;
    uint32_t id;

public:
    TWAICommandBuilderBase(uint32_t id, QueueHandle_t outQ) : outQ(outQ), id(id)
    {
        ESP_LOGI(FUNCTION_NAME, "TWAICommandBuilderBase constructor called");

        msg.extd = 0;
        msg.rtr = 0;
        msg.ss = 0;
        msg.dlc_non_comp = 0;
        setIdentifier();
    }

    void setIdentifier()
    {
        msg.identifier = id;
    }

    void finalize_message(uint8_t *data)
    {
        memcpy(&msg.data, data, msg.data_length_code);
        msg.data[msg.data_length_code - 1] = calculateCRC();
    }

    uint8_t calculateCRC()
    {
        uint8_t crc = msg.identifier;

        for (uint8_t i = 0; i < msg.data_length_code - 1; i++)
        {
            crc += msg.data[i];
        }

        return crc;
    }

    esp_err_t enqueueMessage()
    {
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
