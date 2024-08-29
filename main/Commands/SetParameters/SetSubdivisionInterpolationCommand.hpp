#ifndef SET_SUBDIVISION_INTERPOLATION_COMMAND_H
#define SET_SUBDIVISION_INTERPOLATION_COMMAND_H

#include "../Command.hpp"
#include "../../CANServo.hpp"
#include "esp_log.h"

class SetSubdivisionInterpolationCommand : public Command
{
private:
  CANServo *servo;
  uint8_t enable;

public:
  SetSubdivisionInterpolationCommand(CANServo *servo, uint8_t enable)
      : servo(servo), enable(enable) {}

  void execute() override
  {

    uint8_t data[2];
    data[0] = 0x89; // Set Subdivision Interpolation command code
    data[1] = enable;

    debug.info();
    debug.add("Setting Subdivision Interpolation: ");
    debug.print(enable);

    ESP_LOGI(FUNCTION_NAME, "Setting Subdivision Interpolation: %u", enable);

    servo->sendCommand(data, 2);
  }
};

#endif // SET_SUBDIVISION_INTERPOLATION_COMMAND_H
