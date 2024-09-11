#ifndef QUERYMOTORSTATUSCOMMAND_HPP
#define QUERYMOTORSTATUSCOMMAND_HPP

#include "esp_log.h"

class QueryMotorStatusCommand : public Command
{
private:
    MotorController *servo;

public:
    QueryMotorStatusCommand(MotorController *servo) : servo(servo) {}

    void execute() override
    {

        uint8_t data[1] = {0xF1};

        ESP_LOGI(FUNCTION_NAME, "Querying motor status");

        servo->sendCommand(data, 1);
    }
};
#endif // QUERYMOTORSTATUSCOMMAND_HPP