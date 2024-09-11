#ifndef SET_AUTO_SCREEN_OFF_COMMAND_H
#define SET_AUTO_SCREEN_OFF_COMMAND_H

#include "../../MotorController.hpp"
#include "../Command.hpp"
#include "esp_log.h"

class SetAutoScreenOffCommand : public Command
{
private:
    MotorController *servo;
    uint8_t enable;

public:
    SetAutoScreenOffCommand(MotorController *servo, uint8_t enable)
        : servo(servo), enable(enable) {}

    void execute() override
    {

        uint8_t data[2];
        data[0] = 0x87; // Set Auto Screen Off command code
        data[1] = enable;

        ESP_LOGI(FUNCTION_NAME, "Setting Auto Screen Off: %u", enable);

        servo->sendCommand(data, 2);
    }
};

#endif // SET_AUTO_SCREEN_OFF_COMMAND_H
