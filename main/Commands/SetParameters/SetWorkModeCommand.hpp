#ifndef SET_WORK_MODE_COMMAND_H
#define SET_WORK_MODE_COMMAND_H

#include "../../MotorController.hpp"
#include "../Command.hpp"
#include "esp_log.h"

class SetWorkModeCommand : public Command
{
private:
    MotorController *servo;
    uint8_t mode;

public:
    SetWorkModeCommand(MotorController *servo, uint8_t mode)
        : servo(servo), mode(mode) {}

    void execute() override
    {

        uint8_t data[2];
        data[0] = 0x82; // Set Work Mode command code
        data[1] = mode;

        ESP_LOGI(FUNCTION_NAME, "Setting Work Mode: %u", mode);

        servo->sendCommand(data, 2);
    }
};

#endif // SET_WORK_MODE_COMMAND_H
