#ifndef SET_HOME_COMMAND_H
#define SET_HOME_COMMAND_H

#include "..\Command.hpp"
#include "..\..\CANServo.hpp"
#include "..\..\Debug.hpp"
class SetHomeCommand : public Command
{
private:
  CANServo *servo;
  uint8_t homeTrig;
  uint8_t homeDir;
  uint16_t homeSpeed;
  uint8_t endLimit;

public:
  SetHomeCommand(CANServo *servo, uint8_t homeTrig, uint8_t homeDir, uint16_t homeSpeed, uint8_t endLimit)
      : servo(servo), homeTrig(homeTrig), homeDir(homeDir), homeSpeed(homeSpeed), endLimit(endLimit) {}

  void execute() override
  {
    static const char *TAG = __func__;

    uint8_t data[6];                   // Command code + parameters + CRC
    data[0] = 0x90;                    // Set Home command code
    data[1] = homeTrig;                // Home trigger level: 0 = Low, 1 = High
    data[2] = homeDir;                 // Home direction: 0 = CW, 1 = CCW
    data[3] = (homeSpeed >> 8) & 0xFF; // High byte of home speed
    data[4] = homeSpeed & 0xFF;        // Low byte of home speed
    data[5] = endLimit;                // End limit: 0 = disable, 1 = enable

    debug.info();
    debug.add("ID: ");
    debug.add(servo->getCanId(), 16);
    debug.add(", Home Trigger: ");
    debug.add(homeTrig ? "High" : "Low");
    debug.add(", Home Direction: ");
    debug.add(homeDir ? "CCW" : "CW");
    debug.add(", Home Speed: ");
    debug.add(homeSpeed);
    debug.add(", End Limit: ");
    debug.print(endLimit ? "Enabled" : "Disabled");

    servo->sendCommand(data, 6);
  }
};
#endif // SET_HOME_COMMAND_H