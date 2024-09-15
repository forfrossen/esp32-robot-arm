#ifndef GENERIC_COMMAND_BUILDER_H
#define GENERIC_COMMAND_BUILDER_H

#include "TWAICommandBuilderBase.hpp"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/queue.h" // Add this line to include the header file that defines QueueHandle_t
#include "utils.hpp"
#include <driver/twai.h>
#include <vector>

class GenericCommandBuilder : public TWAICommandBuilderBase
{
private:
    uint8_t commandCode;
    std::vector<uint8_t> payload;

public:
    GenericCommandBuilder(uint32_t id, QueueHandle_t outQ, uint8_t cmdCode, std::vector<uint8_t> payload) : TWAICommandBuilderBase(id, outQ), commandCode(cmdCode), payload(payload)
    {
        msg.data_length_code = 2 + payload.size();
        data = new uint8_t[msg.data_length_code];
    }

    GenericCommandBuilder(uint32_t id, QueueHandle_t outQ, uint8_t cmdCode) : TWAICommandBuilderBase(id, outQ), commandCode(cmdCode)
    {
        msg.data_length_code = 2;
        data = new uint8_t[msg.data_length_code];
    }

    esp_err_t build_twai_message() override
    {
        data[0] = commandCode;
        if (payload.size() > 0)
        {
            set_payload();
        }
        set_msg_data_crc();
        return ESP_OK;
    }

    void set_payload()
    {
        for (int i = 0; i < payload.size(); i++)
        {
            data[i + 1] = payload[i];
        }
    }
};
#endif // GENERIC_COMMAND_BUILDER_H