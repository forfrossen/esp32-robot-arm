#ifndef SET_HOLDING_CURRENT_COMMAND_H
#define SET_HOLDING_CURRENT_COMMAND_H

#include "..\Command.hpp"
#include "../../CANServo.hpp"
#include "esp_log.h"

class SetHoldingCurrentCommand : public Command
{
private:
  CANServo *servo;
  uint8_t holdCurrent;

public:
  SetHoldingCurrentCommand(CANServo *servo, uint8_t holdCurrent)
      : servo(servo), holdCurrent(holdCurrent) {}

  void execute() override
  {

    uint8_t data[2];
    data[0] = 0x9B; // Set Holding Current command code
    data[1] = holdCurrent;

    ESP_LOGI(FUNCTION_NAME, "Setting Holding Current: %u", holdCurrent);

    servo->sendCommand(data, 2);
  }
};

#endif // SET_HOLDING_CURRENT_COMMAND_H
