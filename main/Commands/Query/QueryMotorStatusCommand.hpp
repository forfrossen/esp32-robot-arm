#ifndef QUERYMOTORSTATUSCOMMAND_HPP
#define QUERYMOTORSTATUSCOMMAND_HPP

#include "esp_log.h"

class QueryMotorStatusCommand : public Command
{
private:
  CANServo *servo;

public:
  QueryMotorStatusCommand(CANServo *servo) : servo(servo) {}

  void execute() override
  {

    static const char *TAG = __func__;
    uint8_t data[1] = {0xF1};

    ESP_LOGI(TAG, "Querying motor status");

    servo->sendCommand(data, 1);
  }
};
#endif // QUERYMOTORSTATUSCOMMAND_HPP