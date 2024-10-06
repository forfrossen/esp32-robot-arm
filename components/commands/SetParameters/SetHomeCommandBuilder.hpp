#ifndef SET_HOME_COMMAND_BUILDER_H
#define SET_HOME_COMMAND_BUILDER_H

#include "../TypeDefs.hpp"
#include "TWAICommandBuilderBase.hpp"
#include "esp_log.h"
#include "freertos/queue.h"

class SetHomeParametersCommandBuilder : public TWAICommandBuilderBase<SetHomeParametersCommandBuilder>
{
private:
    uint8_t direction;
    uint8_t speed;
    uint8_t acceleration;
    uint8_t end_limit;
    uint8_t home_trigger;
    bool absolute;

public:
    SetHomeParametersCommandBuilder(uint32_t id, QueueHandle_t outQ, QueueHandle_t inQ) : TWAICommandBuilderBase(id, outQ, inQ)
    {
        msg.data_length_code = 8;
        data = new uint8_t[msg.data_length_code];
    }

    SetHomeParametersCommandBuilder &set_end_limit(uint8_t end_limit)
    {
        this->end_limit = end_limit;
        return *this;
    }

    SetHomeParametersCommandBuilder &set_direction(uint8_t direction)
    {
        this->direction = direction;
        return *this;
    }

    SetHomeParametersCommandBuilder &set_speed(uint8_t speed)
    {
        this->speed = speed;
        return *this;
    }

    SetHomeParametersCommandBuilder &set_acceleration(uint8_t acceleration)
    {
        this->acceleration = acceleration;
        return *this;
    }

    SetHomeParametersCommandBuilder &set_absolute(bool absolute)
    {
        this->absolute = absolute;
        return *this;
    }

    esp_err_t build_twai_message()
    {
        if (speed > 3000)
        {
            speed = 3000;
        }

        uint8_t data[6];               // Command code + parameters + CRC
        data[0] = 0x90;                // Set Home command code
        data[1] = home_trigger;        // Home trigger level: 0 = Low, 1 = High
        data[2] = direction;           // Home direction: 0 = CW, 1 = CCW
        data[3] = (speed >> 8) & 0xFF; // High byte of home speed
        data[4] = speed & 0xFF;        // Low byte of home speed
        data[5] = end_limit;           // End limit: 0 = disable, 1 = enable

        ESP_LOGI(FUNCTION_NAME, "Setting Home: %s, %s, %u RPM, %s", home_trigger ? "High" : "Low", direction ? "CCW" : "CW", speed, end_limit ? "Enabled" : "Disabled");

        return ESP_OK;
    }
};

#endif