#ifndef RUN_MOTOR_IN_SPEED_MODE_COMMAND_BUILDER_H
#define RUN_MOTOR_IN_SPEED_MODE_COMMAND_BUILDER_H

#include "../TypeDefs.hpp"
#include "TWAICommandBuilderBase.hpp"
#include "esp_log.h"
#include "freertos/queue.h"

class RunMotorInSpeedModeCommandBuilder : public TWAICommandBuilderBase<RunMotorInSpeedModeCommandBuilder>
{
private:
    uint8_t direction;
    uint16_t speed;
    uint8_t acceleration;

public:
    RunMotorInSpeedModeCommandBuilder(std::shared_ptr<TWAICommandFactorySettings> settings) : TWAICommandBuilderBase<RunMotorInSpeedModeCommandBuilder>(settings, 0xF6)
    {
    }
    ~RunMotorInSpeedModeCommandBuilder()
    {
        ESP_LOGW(FUNCTION_NAME, "RunMotorInSpeedModeCommandBuilder destructor called");
        delete[] data;
    }

    RunMotorInSpeedModeCommandBuilder &init_new_command()
    {
        set_command_code(0xF6);
        set_data_length_code(4);
        create_msg_data();
        return *this;
    }

    RunMotorInSpeedModeCommandBuilder &set_direction(uint8_t direction)
    {
        this->direction = direction;
        return *this;
    }

    RunMotorInSpeedModeCommandBuilder &set_speed(uint16_t speed)
    {
        this->speed = speed;
        return *this;
    }

    RunMotorInSpeedModeCommandBuilder &set_acceleration(uint8_t acceleration)
    {
        this->acceleration = acceleration;
        return *this;
    }

    esp_err_t build_twai_message()
    {
        if (speed > 3000)
        {
            speed = 3000;
        }

        data[0] = command_code;
        data[1] = (direction ? 0x80 : 0x00) | ((speed >> 8) & 0x0F);
        data[2] = speed & 0xFF;
        data[3] = acceleration;

        ESP_LOGI(FUNCTION_NAME, "Running motor in speed mode: %u RPM, acceleration: %u, direction: %s", speed, acceleration, direction ? "CW" : "CCW");

        return ESP_OK;
    }
};

#endif // RUN_MOTOR_IN_SPEED_MODE_COMMAND_BUILDER_H