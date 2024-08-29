#ifndef SET_SUBDIVISION_COMMAND_H
#define SET_SUBDIVISION_COMMAND_H

#include "../Command.hpp"
#include "../../CANServo.hpp"
#include "esp_log.h"

class SetSubdivisionCommand : public Command
{
private:
  CANServo *servo;
  uint8_t subdivision;

public:
  SetSubdivisionCommand(CANServo *servo, uint8_t subdivision)
      : servo(servo), subdivision(subdivision) {}

  void execute() override
  {

    uint8_t data[2];
    data[0] = 0x84; // Set Subdivision command code
    data[1] = subdivision;

    ESP_LOGI(FUNCTION_NAME, "Setting Subdivision: %u", subdivision);

    servo->sendCommand(data, 2);
  }
};

#endif // SET_SUBDIVISION_COMMAND_H
