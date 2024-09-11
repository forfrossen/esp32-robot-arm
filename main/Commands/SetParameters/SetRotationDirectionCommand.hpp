#ifndef SET_ROTATION_DIRECTION_COMMAND_H
#define SET_ROTATION_DIRECTION_COMMAND_H

#include "../../MotorController.hpp"
#include "../Command.hpp"
#include "esp_log.h"

class SetRotationDirectionCommand : public Command
{
private:
    MotorController *servo;
    uint8_t direction;

public:
    SetRotationDirectionCommand(MotorController *servo, uint8_t direction)
        : servo(servo), direction(direction) {}

    void execute() override
    {

        uint8_t data[2];
        data[0] = 0x86; // Set Rotation Direction command code
        data[1] = direction;

        ESP_LOGI(FUNCTION_NAME, "Setting Rotation Direction: %u", direction);

        servo->sendCommand(data, 2);
    }
};

#endif // SET_ROTATION_DIRECTION_COMMAND_H
