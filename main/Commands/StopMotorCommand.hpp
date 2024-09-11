#ifndef STOP_MOTOR_COMMAND_H
#define STOP_MOTOR_COMMAND_H

#include "../MotorController.hpp"
#include "Command.hpp"
#include "esp_log.h"

class StopMotorCommand : public Command
{
private:
    MotorController *servo;

public:
    StopMotorCommand(MotorController *servo) : servo(servo) {}

    void execute() override
    {

        uint8_t data[4] = {0xF7};

        ESP_LOGI(FUNCTION_NAME, "Stopping motor");

        servo->sendCommand(data, 1);
        servo->setState(StateMachine::State::REQUESTED);
    }
};

#endif // STOP_MOTOR_COMMAND_H