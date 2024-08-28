#ifndef SET_ROTATION_DIRECTION_COMMAND_H
#define SET_ROTATION_DIRECTION_COMMAND_H

#include "../Command.hpp"
#include "../../CANServo.hpp"
#include "esp_log.h"

class SetRotationDirectionCommand : public Command
{
private:
  CANServo *servo;
  uint8_t direction;

public:
  SetRotationDirectionCommand(CANServo *servo, uint8_t direction)
      : servo(servo), direction(direction) {}

  void execute() override
  {
    static const char *TAG = FUNCTION_NAME;
    uint8_t data[2];
    data[0] = 0x86; // Set Rotation Direction command code
    data[1] = direction;

    ESP_LOGI(TAG, "Setting Rotation Direction: %u", direction);

    servo->sendCommand(data, 2);
  }
};

#endif // SET_ROTATION_DIRECTION_COMMAND_H
