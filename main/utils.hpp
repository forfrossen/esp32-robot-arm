#ifndef UTILS_HPP
#define UTILS_HPP

#include "MksEnums.hpp"
#include <algorithm>
#include <cstring>
#include <driver/twai.h>
#include <string>

inline const char *findFunctionNameStart(const char *prettyFunction)
{
    const char *start = prettyFunction;
    const char *lastSpace = nullptr;
    const char *doubleColon = strstr(prettyFunction, "::");

    while (*start != '\0' && start < doubleColon)
    {
        if (*start == ' ')
        {
            lastSpace = start;
        }
        ++start;
    }

    if (lastSpace)
    {
        start = lastSpace + 1; // Move past the last space
    }
    else
    {
        start = prettyFunction; // No space found, start from the beginning
    }

    return start;
}

inline const char *findFunctionNameEnd(const char *start)
{
    const char *end = start;
    while (*end != '(')
        ++end; // Find the opening parenthesis
    return end;
}

inline const char *getFunctionName(const char *prettyFunction)
{
    const char *start = findFunctionNameStart(prettyFunction);
    const char *end = findFunctionNameEnd(start);

    static thread_local char functionName[256];
    size_t length = end - start;

    if (length >= sizeof(functionName))
    {
        length = sizeof(functionName) - 1;
    }
    strncpy(functionName, start, length);
    functionName[length] = '\0';
    return functionName;
}

#define FUNCTION_NAME getFunctionName(__PRETTY_FUNCTION__)

#define GET_CMD(msg) static_cast<CommandIds>((msg)->data[0])

#define COMMAND_TO_STRING_CASE(cmd) \
    case cmd:                       \
        return #cmd;

inline std::string replace_underscores(const char *str)
{
    std::string result(str);
    std::replace(result.begin(), result.end(), '_', ' ');
    return result;
}

inline const char *get_name_for_cmd(CommandIds cmd)
{
    switch (cmd)
    {
        COMMAND_TO_STRING_CASE(CommandIds::MOTOR_CALIBRATION)
        COMMAND_TO_STRING_CASE(CommandIds::SET_WORK_MODE)
        COMMAND_TO_STRING_CASE(CommandIds::SET_WORKING_CURRENT)
        COMMAND_TO_STRING_CASE(CommandIds::SET_HOLDING_CURRENT)
        COMMAND_TO_STRING_CASE(CommandIds::SET_SUBDIVISIONS)
        COMMAND_TO_STRING_CASE(CommandIds::SET_EN_PIN_CONFIG)
        COMMAND_TO_STRING_CASE(CommandIds::SET_MOTOR_ROTATION_DIRECTION)
        COMMAND_TO_STRING_CASE(CommandIds::SET_AUTO_TURN_OFF_SCREEN)
        COMMAND_TO_STRING_CASE(CommandIds::SET_MOTOR_SHAFT_LOCKED_ROTOR_PROTECTION)
        COMMAND_TO_STRING_CASE(CommandIds::SET_SUBDIVISION_INTERPOLATION)
        COMMAND_TO_STRING_CASE(CommandIds::SET_CAN_BITRATE)
        COMMAND_TO_STRING_CASE(CommandIds::SET_CAN_ID)
        COMMAND_TO_STRING_CASE(CommandIds::SET_KEY_LOCK_ENABLE)
        COMMAND_TO_STRING_CASE(CommandIds::SET_GROUP_ID)
        COMMAND_TO_STRING_CASE(CommandIds::SET_HOME)
        COMMAND_TO_STRING_CASE(CommandIds::GO_HOME)
        COMMAND_TO_STRING_CASE(CommandIds::SET_CURRENT_AXIS_TO_ZERO)
        COMMAND_TO_STRING_CASE(CommandIds::SET_LIMIT_PORT_REMAP)
        COMMAND_TO_STRING_CASE(CommandIds::SET_MODE0)
        COMMAND_TO_STRING_CASE(CommandIds::RESTORE_DEFAULT_PARAMETERS)
        COMMAND_TO_STRING_CASE(CommandIds::READ_ENCODER_VALUE_CARRY)
        COMMAND_TO_STRING_CASE(CommandIds::READ_ENCODED_VALUE_ADDITION)
        COMMAND_TO_STRING_CASE(CommandIds::READ_MOTOR_SPEED)
        COMMAND_TO_STRING_CASE(CommandIds::READ_NUM_PULSES_RECEIVED)
        COMMAND_TO_STRING_CASE(CommandIds::READ_IO_PORT_STATUS)
        COMMAND_TO_STRING_CASE(CommandIds::READ_MOTOR_SHAFT_ANGLE_ERROR)
        COMMAND_TO_STRING_CASE(CommandIds::READ_EN_PINS_STATUS)
        COMMAND_TO_STRING_CASE(CommandIds::READ_GO_BACK_TO_ZERO_STATUS_WHEN_POWER_ON)
        COMMAND_TO_STRING_CASE(CommandIds::RELEASE_MOTOR_SHAFT_LOCKED_PROTECTION_STATE)
        COMMAND_TO_STRING_CASE(CommandIds::READ_MOTOR_SHAFT_PROTECTION_STATE)
        COMMAND_TO_STRING_CASE(CommandIds::QUERY_MOTOR_STATUS)
        COMMAND_TO_STRING_CASE(CommandIds::ENABLE_MOTOR)
        COMMAND_TO_STRING_CASE(CommandIds::EMERGENCY_STOP)
        COMMAND_TO_STRING_CASE(CommandIds::RUN_MOTOR_SPEED_MODE)
        COMMAND_TO_STRING_CASE(CommandIds::SAVE_CLEAN_IN_SPEED_MODE)
        COMMAND_TO_STRING_CASE(CommandIds::RUN_MOTOR_RELATIVE_MOTION_BY_PULSES)
        COMMAND_TO_STRING_CASE(CommandIds::RUN_MOTOR_ABSOLUTE_MOTION_BY_PULSES)
        COMMAND_TO_STRING_CASE(CommandIds::RUN_MOTOR_RELATIVE_MOTION_BY_AXIS)
        COMMAND_TO_STRING_CASE(CommandIds::RUN_MOTOR_ABSOLUTE_MOTION_BY_AXIS)
    default:
        return "Unknown command";
    }
}

inline std::string GET_CMD_NAME(CommandIds cmd)
{
    return replace_underscores(get_name_for_cmd(cmd));
}

#endif // UTILS_HPP
