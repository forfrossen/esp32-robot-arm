#ifndef RUN_MOTOR_IN_SPEED_MODE_COMMAND_H
#define RUN_MOTOR_IN_SPEED_MODE_COMMAND_H

#include "Command.hpp"
#include "..\CANServo.hpp"
#include "esp_log.h"

class RunMotorInSpeedModeCommand : public Command
{
private:
  CANServo *servo;
  bool direction;
  uint16_t speed;
  uint8_t acceleration;

public:
  RunMotorInSpeedModeCommand(CANServo *servo, bool direction, uint16_t speed, uint8_t acceleration)
      : servo(servo), direction(direction), speed(speed), acceleration(acceleration) {}

  void execute() override
  {
    static const char *TAG = FUNCTION_NAME;

    if (speed > 3000)
    {
      speed = 3000;
    }

    uint8_t data[3];
    data[0] = 0xF6;
    data[1] = (direction ? 0x80 : 0x00) | ((speed >> 8) & 0x0F);
    data[2] = speed & 0xFF;
    data[3] = acceleration;

    ESP_LOGI(TAG, "Running motor in speed mode: %u RPM, acceleration: %u, direction: %s", speed, acceleration, direction ? "CW" : "CCW");

    servo->sendCommand(data, 3);
  }
};
#endif // RUN_MOTOR_IN_SPEED_MODE_COMMAND_H