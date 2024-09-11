#ifndef SET_CAN_BITRATE_COMMAND_H
#define SET_CAN_BITRATE_COMMAND_H

#include "../../MotorController.hpp"
#include "../Command.hpp"
#include "esp_log.h"

class SetCANBitRateCommand : public Command
{
private:
    MotorController *servo;
    uint8_t bitRate;

public:
    SetCANBitRateCommand(MotorController *servo, uint8_t bitRate)
        : servo(servo), bitRate(bitRate) {}

    void execute() override
    {

        uint8_t data[2];
        data[0] = 0x8A; // Set CAN Bit Rate command code
        data[1] = bitRate;

        ESP_LOGI(FUNCTION_NAME, "Setting CAN Bit Rate: %u", bitRate);

        servo->sendCommand(data, 2);
    }
};

#endif // SET_CAN_BITRATE_COMMAND_H
