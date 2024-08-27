#ifndef RUN_MOTOR_IN_SPEED_MODE_COMMAND_H
#define RUN_MOTOR_IN_SPEED_MODE_COMMAND_H

#include "Command.hpp"
#include "..\CANServo.hpp"
#include "..\Debug.hpp"

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
    static const char *TAG = __func__;
    if (speed > 3000)
    {
      speed = 3000;
    }

    uint8_t data[3];
    data[0] = 0xF6;
    data[1] = (direction ? 0x80 : 0x00) | ((speed >> 8) & 0x0F);
    data[2] = speed & 0xFF;
    data[3] = acceleration;

    debug.info();
    debug.add("Running motor in speed mode: ");
    debug.add(speed);
    debug.add(" RPM, acceleration: ");
    debug.add(acceleration);
    debug.add(", direction: ");
    debug.print(direction ? "CW" : "CCW");

    servo->sendCommand(data, 3);
  }
};
#endif // RUN_MOTOR_IN_SPEED_MODE_COMMAND_H