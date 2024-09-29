#include "CommandMapper.hpp"

// Initialisierung der statischen konstanten Map
const std::unordered_map<uint8_t, const char *> CommandMapper::command_map = {
    {0x30, "Read encoder value (carry)"},
    {0x31, "Read encoder value (addition)"},
    {0x32, "Read motor speed"},
    {0x33, "Read number of pulses"},
    {0x34, "Read IO Ports status"},
    {0x39, "Read motor shaft angle error"},
    {0x3A, "Read En pin status"},
    {0x3B, "Read go back to zero status"},
    {0x3D, "Release motor shaft locked-rotor protection state"},
    {0x3E, "Read motor shaft protection state"},
    {0x3F, "Restore default parameters"},
    {0x80, "Calibrate encoder"},
    {0x82, "Set work mode"},
    {0x83, "Set working current"},
    {0x84, "Set subdivisions"},
    {0x85, "Set active level of En pin"},
    {0x86, "Set motor rotation direction"},
    {0x87, "Set auto turn off screen"},
    {0x88, "Set motor shaft locked-rotor protection function"},
    {0x89, "Set subdivision interpolation function"},
    {0x8A, "Set CAN bitRate"},
    {0x8B, "Set CAN ID"},
    {0x8D, "Set group ID"},
    {0x8F, "Set key lock/unlock"},
    {0x90, "Set parameters of home"},
    {0x91, "Go home"},
    {0x92, "Set current axis to zero"},
    {0x94, "Set parameter of 'noLimit' go home"},
    {0x9A, "Set Mode0"},
    {0x9B, "Set holding current"},
    {0x9E, "Set limit port remap"},
    {0xF1, "Query motor status"},
    {0xF2, "Query motor position"},
    {0xF3, "Enable the motor"},
    {0xF4, "Relative position control mode"},
    {0xF5, "Absolute position control mode"},
    {0xF6, "Speed mode command"},
    {0xF7, "Emergency stop"},
    {0xFD, "Run motor relative motion by pulses"},
    {0xFE, "Run motor absolute motion by pulses"},
    {0xFF, "Save/Clean in speed mode"}};

CommandMapper::CommandMapper() {}

void CommandMapper::get_command_name_from_code(uint8_t code, char *command_name) const
{
    ESP_LOGI(FUNCTION_NAME, "Looking for code: 0x%02X", code);

    auto it = command_map.find(code);
    if (it != command_map.end())
    {
        std::strcpy(command_name, it->second);
        ESP_LOGI(FUNCTION_NAME, "Found command name: %s", command_name);
    }
    else
    {
        const char *unknown_command = "Unknown command";
        std::strcpy(command_name, unknown_command);
        ESP_LOGE(FUNCTION_NAME, "Command not found, using unknown command");
    }
}
