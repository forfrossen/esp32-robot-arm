#ifndef SET_identifier_COMMAND_H
#define SET_identifier_COMMAND_H

#include "../Command.hpp"
#include "../../CANServo.hpp"
#include "esp_log.h"

class SetCANIDCommand : public Command
{
private:
  CANServo *servo;
  uint16_t canId;

public:
  SetCANIDCommand(CANServo *servo, uint16_t canId)
      : servo(servo), canId(canId) {}

  void execute() override
  {

    uint8_t data[3];
    data[0] = 0x8B; // Set CAN ID command code
    data[1] = (canId >> 8) & 0xFF;
    data[2] = canId & 0xFF;

    ESP_LOGI(FUNCTION_NAME, "Setting CAN ID: %u", canId);

    servo->sendCommand(data, 3);
  }
};

#endif // SET_identifier_COMMAND_H
