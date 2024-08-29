#ifndef QUERYMOTORPOSITIONCOMMAND_HPP
#define QUERYMOTORPOSITIONCOMMAND_HPP

#include "esp_log.h"

class QueryMotorPositionCommand : public Command
{
private:
  CANServo *servo;

public:
  QueryMotorPositionCommand(CANServo *servo) : servo(servo) {}

  void execute() override
  {

    uint8_t data[1] = {0x30};

    ESP_LOGI(FUNCTION_NAME, "Querying motor position");

    servo->sendCommand(data, 1);
  }
};
#endif // QUERYMOTORPOSITIONCOMMAND_HPP