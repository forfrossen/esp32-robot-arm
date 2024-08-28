#ifndef SET_ENABLE_PIN_COMMAND_H
#define SET_ENABLE_PIN_COMMAND_H

#include "../Command.hpp"
#include "../../CANServo.hpp"
#include "esp_log.h"

class SetEnablePinCommand : public Command
{
private:
  CANServo *servo;
  uint8_t enable;

public:
  SetEnablePinCommand(CANServo *servo, uint8_t enable)
      : servo(servo), enable(enable) {}

  void execute() override
  {
    static const char *TAG = FUNCTION_NAME;
    uint8_t data[2];
    data[0] = 0x85; // Set Enable Pin command code
    data[1] = enable;

    ESP_LOGI(TAG, "Setting Enable Pin: %u", enable);

    servo->sendCommand(data, 2);
  }
};

#endif // SET_ENABLE_PIN_COMMAND_H
