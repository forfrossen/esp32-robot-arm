#ifndef COMMANDMAPPER_H
#define COMMANDMAPPER_H
#include <map>
#include <string>
#include <cstdint>
#include <cstring>
#include "utils.hpp"

struct CommandMap
{
  uint8_t commandNum;
  const char *commandName;
};

static const char cmd0[] = "Read encoder value (carry)";
static const char cmd1[] = "Read encoder value (addition)";
static const char cmd2[] = "Read motor speed";
static const char cmd3[] = "Read number of pulses";
static const char cmd4[] = "Read IO Ports status";
static const char cmd5[] = "Read motor shaft angle error";
static const char cmd6[] = "Read En pin status";
static const char cmd7[] = "Read go back to zero status";
static const char cmd8[] = "Release motor shaft locked-rotor protection state";
static const char cmd9[] = "Read motor shaft protection state";
static const char cmd10[] = "Calibrate encoder";
static const char cmd11[] = "Set work mode";
static const char cmd12[] = "Set working current";
static const char cmd13[] = "Set subdivisions";
static const char cmd14[] = "Set active level of En pin";
static const char cmd15[] = "Set motor rotation direction";
static const char cmd16[] = "Set auto turn off screen";
static const char cmd17[] = "Set motor shaft locked-rotor protection function";
static const char cmd18[] = "Set subdivision interpolation function";
static const char cmd19[] = "Set CAN bitRate";
static const char cmd20[] = "Set CAN ID";
static const char cmd21[] = "Set slave respond and active";
static const char cmd22[] = "Set group ID";
static const char cmd23[] = "Set key lock/unlock";
static const char cmd24[] = "Set parameters of home";
static const char cmd25[] = "Go home";
static const char cmd26[] = "Set current axis to zero";
static const char cmd27[] = "Set parameter of 'noLimit' go home";
static const char cmd28[] = "Query motor status";
static const char cmd29[] = "Query motor position";
static const char cmd30[] = "Enable the motor";
static const char cmd31[] = "Relative position control mode";
static const char cmd32[] = "Absolute position control mode";
static const char cmd33[] = "Speed mode command";
static const char cmd34[] = "Stop motor";
static const char unknownCommand[] = "Unknown command";

class CommandMapper
{

private:
  std::map<uint8_t, const char *>
      commandMap;

public:
  CommandMapper()
  {
    commandMap = {
        {0x30, cmd0},
        {0x31, cmd1},
        {0x32, cmd2},
        {0x33, cmd3},
        {0x34, cmd4},
        {0x39, cmd5},
        {0x3A, cmd6},
        {0x3B, cmd7},
        {0x3D, cmd8},
        {0x3E, cmd9},
        {0x80, cmd10},
        {0x82, cmd11},
        {0x83, cmd12},
        {0x84, cmd13},
        {0x85, cmd14},
        {0x86, cmd15},
        {0x87, cmd16},
        {0x88, cmd17},
        {0x89, cmd18},
        {0x8A, cmd19},
        {0x8B, cmd20},
        {0x8C, cmd21},
        {0x8D, cmd22},
        {0x8F, cmd23},
        {0x90, cmd24},
        {0x91, cmd25},
        {0x92, cmd26},
        {0x94, cmd27},
        {0xF1, cmd28},
        {0xF2, cmd29},
        {0xF3, cmd30},
        {0xF4, cmd31},
        {0xF5, cmd32},
        {0xF6, cmd33},
        {0xF7, cmd34},
        {0xFF, unknownCommand}};
  }

  // Declaration of the getCommandNameFromCode function
  void getCommandNameFromCode(uint8_t code, char *commandName)
  {
    static const char *TAG = FUNCTION_NAME;

    ESP_LOGI(TAG, "Looking for code: 0x%02X", code);

    auto it = commandMap.find(code);
    if (it != commandMap.end())
    {
      std::strcpy(commandName, it->second);
      ESP_LOGI(TAG, "Found command name: %s", commandName);
    }
    else
    {
      std::strcpy(commandName, unknownCommand);
      ESP_LOGE(TAG, "Command not found, using unknown command");
    }
  };
};
#endif