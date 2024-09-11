#ifndef SET_CURRENT_COMMAND_H
#define SET_CURRENT_COMMAND_H

#include "../../MotorController.hpp"
#include "../Command.hpp"
#include "esp_log.h"

class SetCurrentCommand : public Command
{
private:
    MotorController *servo;
    uint16_t current;

public:
    SetCurrentCommand(MotorController *servo, uint16_t current)
        : servo(servo), current(current) {}

    void execute() override
    {

        uint8_t data[3];
        data[0] = 0x83; // Set Current command code
        data[1] = (current >> 8) & 0xFF;
        data[2] = current & 0xFF;

        ESP_LOGI(FUNCTION_NAME, "Setting Current: %u", current);

        servo->sendCommand(data, 3);
    }
};

#endif // SET_CURRENT_COMMAND_H
