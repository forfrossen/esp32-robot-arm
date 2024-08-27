#ifndef STOP_MOTOR_COMMAND_H
#define STOP_MOTOR_COMMAND_H

#include "Command.hpp"
#include "..\CANServo.hpp"
#include "..\Debug.hpp"

class StopMotorCommand : public Command
{
private:
  CANServo *servo;

public:
  StopMotorCommand(CANServo *servo) : servo(servo) {}

  void execute() override
  {
    static const char *TAG = __func__;
    uint8_t data[4] = {0xF7};

    debug.info();
    debug.print("Stopping motor");

    servo->sendCommand(data, 1);
  }
};

#endif // STOP_MOTOR_COMMAND_H