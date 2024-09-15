#ifndef RUN_MOTOR_IN_SPEED_MODE_COMMAND_BUILDER_H
#define RUN_MOTOR_IN_SPEED_MODE_COMMAND_BUILDER_H

#include "TWAICommandBuilderBase.hpp"
#include "esp_log.h"
#include "freertos/queue.h"

class RunMotorInSpeedModeCommandBuilder : public TWAICommandBuilderBase
{
private:
    uint8_t direction;
    uint8_t speed;
    uint8_t acceleration;
    bool absolute;

public:
    RunMotorInSpeedModeCommandBuilder(uint32_t id, QueueHandle_t outQ) : TWAICommandBuilderBase(id, outQ)
    {
        msg.data_length_code = 8;
        data = new uint8_t[msg.data_length_code];
    }

    RunMotorInSpeedModeCommandBuilder &set_direction(uint8_t direction)
    {
        this->direction = direction;
        return *this;
    }

    RunMotorInSpeedModeCommandBuilder &set_speed(uint8_t speed)
    {
        this->speed = speed;
        return *this;
    }

    RunMotorInSpeedModeCommandBuilder &set_acceleration(uint8_t acceleration)
    {
        this->acceleration = acceleration;
        return *this;
    }

    RunMotorInSpeedModeCommandBuilder &set_absolute(bool absolute)
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

        uint8_t data[3];
        data[0] = 0xF6;
        data[1] = (direction ? 0x80 : 0x00) | ((speed >> 8) & 0x0F);
        data[2] = speed & 0xFF;
        data[3] = acceleration;

        ESP_LOGI(FUNCTION_NAME, "Running motor in speed mode: %u RPM, acceleration: %u, direction: %s", speed, acceleration, direction ? "CW" : "CCW");

        return ESP_OK;
    }
};

#endif // RUN_MOTOR_IN_SPEED_MODE_COMMAND_BUILDER_H