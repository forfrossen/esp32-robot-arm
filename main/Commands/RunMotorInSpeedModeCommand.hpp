#ifndef RUN_MOTOR_IN_SPEED_MODE_COMMAND_H
#define RUN_MOTOR_IN_SPEED_MODE_COMMAND_H

#include "..\MotorController.hpp"
#include "Command.hpp"
#include "esp_log.h"

class RunMotorInSpeedModeCommand : public Command
{
private:
    MotorController *servo;
    bool direction;
    uint16_t speed;
    uint8_t acceleration;

public:
    RunMotorInSpeedModeCommand(MotorController *servo, bool direction, uint16_t speed, uint8_t acceleration)
        : servo(servo), direction(direction), speed(speed), acceleration(acceleration) {}

    void execute() override
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

        servo->sendCommand(data, 3);
        servo->setState(StateMachine::State::REQUESTED);
    }
};
#endif // RUN_MOTOR_IN_SPEED_MODE_COMMAND_H