#ifndef SET_HOLDING_CURRENT_COMMAND_BUILDER_H
#define SET_HOLDING_CURRENT_COMMAND_BUILDER_H

#include "TWAICommandBuilderBase.hpp"
#include "esp_log.h"
#include "freertos/queue.h"

class SetHoldingCurrentCommandBuilder : public TWAICommandBuilderBase
{
private:
    uint16_t holdMa;
    bool absolute;

public:
    SetHoldingCurrentCommandBuilder(uint32_t id, QueueHandle_t outQ, QueueHandle_t inQ) : TWAICommandBuilderBase(id, outQ, inQ)
    {
        msg.data_length_code = 8;
        data = new uint8_t[msg.data_length_code];
    }

    // holdMa = 00 10%
    // holdMa = 01 20%
    // holdMa = 02 30%
    // holdMa = 03 40%
    // holdMa = 04 50%
    // holdMa = 05 60%
    // holdMa = 06 70%
    // holdMa = 07 80%
    // holdMa = 08 90%
    SetHoldingCurrentCommandBuilder &set_position(uint16_t holdMa)
    {
        this->holdMa = holdMa;
        return *this;
    }

    esp_err_t build_twai_message()
    {
        uint8_t data[3];
        data[0] = 0x83; // Set Current command code
        data[1] = (holdMa >> 8) & 0xFF;
        data[2] = holdMa & 0xFF;

        ESP_LOGI(FUNCTION_NAME, "Setting Current: %u", holdMa);

        return ESP_OK;
    }
};

#endif // SET_TARGET_POSITION_COMMAND_H