#ifndef RUN_MOTOR_IN_SPEED_MODE_COMMAND_BUILDER_H
#define RUN_MOTOR_IN_SPEED_MODE_COMMAND_BUILDER_H

#include "CommandBase.hpp"
#include "TypeDefs.hpp"
#include <esp_log.h>
#include <freertos/queue.h>

class RunMotorInSpeedModeCommand : public CommandBase<RunMotorInSpeedModeCommand>
{
private:
    uint8_t direction;
    uint16_t speed;
    uint8_t acceleration;

public:
    RunMotorInSpeedModeCommand(std::shared_ptr<CommandFactorySettings> settings) : CommandBase<RunMotorInSpeedModeCommand>(settings, RUN_MOTOR_SPEED_MODE)
    {
    }
    ~RunMotorInSpeedModeCommand()
    {
        ESP_LOGW(FUNCTION_NAME, "RunMotorInSpeedModeCommand destructor called");
        delete[] data;
    }

    RunMotorInSpeedModeCommand &init_new_command()
    {
        set_command_id(RUN_MOTOR_SPEED_MODE);
        // set_data_length_code(4);
        create_msg_data();
        return *this;
    }

    RunMotorInSpeedModeCommand &set_direction(uint8_t direction)
    {
        this->direction = direction;
        return *this;
    }

    RunMotorInSpeedModeCommand &set_speed(uint16_t speed)
    {
        this->speed = speed;
        return *this;
    }

    RunMotorInSpeedModeCommand &set_acceleration(uint8_t acceleration)
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

        data[0] = command_id;
        data[1] = (direction ? 0x80 : 0x00) | ((speed >> 8) & 0x0F);
        data[2] = speed & 0xFF;
        data[3] = acceleration;

        ESP_LOGD(FUNCTION_NAME, "Running motor in speed mode: %u RPM, acceleration: %u, direction: %s", speed, acceleration, direction ? "CW" : "CCW");

        return ESP_OK;
    }
};

#endif // RUN_MOTOR_IN_SPEED_MODE_COMMAND_BUILDER_H