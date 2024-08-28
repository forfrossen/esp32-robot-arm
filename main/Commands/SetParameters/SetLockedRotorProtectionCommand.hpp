#ifndef SET_LOCKED_ROTOR_PROTECTION_COMMAND_H
#define SET_LOCKED_ROTOR_PROTECTION_COMMAND_H

#include "../Command.hpp"
#include "../../CANServo.hpp"
#include "esp_log.h"

class SetLockedRotorProtectionCommand : public Command
{
private:
  CANServo *servo;
  uint8_t enable;

public:
  SetLockedRotorProtectionCommand(CANServo *servo, uint8_t enable)
      : servo(servo), enable(enable) {}

  void execute() override
  {
    static const char *TAG = FUNCTION_NAME;
    uint8_t data[2];
    data[0] = 0x88; // Set Locked Rotor Protection command code
    data[1] = enable;

    ESP_LOGI(TAG, "Setting Locked Rotor Protection: %u", enable);

    servo->sendCommand(data, 2);
  }
};

#endif // SET_LOCKED_ROTOR_PROTECTION_COMMAND_H
