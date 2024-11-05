#ifndef COMMAND_PAYLOAD_TYPEDEFS_HPP
#define COMMAND_PAYLOAD_TYPEDEFS_HPP

#include "MksEnums.hpp"
#include <algorithm>
#include <array>
#include <esp_event.h>
#include <esp_log.h>
#include <map>
#include <memory>
#include <optional>
#include <type_traits>
#include <variant>

struct CommandPayloadInfo
{
    std::array<PayloadType, 7> type_info;

    template <typename... Types>
    CommandPayloadInfo(Types... types)
    {
        static_assert(sizeof...(types) <= 7, "Too many payload types provided, max is 7");

        static_assert((... && std::is_same_v<Types, PayloadType>),
                      "All arguments must be of type PayloadType");
        std::size_t index = 0;

        ((type_info[index++] = types), ...);

        while (index < type_info.size())
        {
            type_info[index++] = PayloadType::VOID;
        }
    }

    PayloadType get_type(std::size_t index) const
    {
        if (index < type_info.size())
        {
            return type_info[index];
        }
        else
        {
            return PayloadType::VOID;
        }
    }

    size_t get_size() const
    {
        return std::count_if(type_info.begin(), type_info.end(), [](PayloadType type)
                             { return type != PayloadType::VOID; });
    }
};

const std::map<motor_command_id_t, CommandPayloadInfo> g_command_payload_map = {
    {MOTOR_CALIBRATION, CommandPayloadInfo()},
    {READ_MOTOR_SPEED, CommandPayloadInfo()},
    {EMERGENCY_STOP, CommandPayloadInfo()},
    {READ_ENCODER_VALUE_CARRY, CommandPayloadInfo()},
    {READ_ENCODED_VALUE_ADDITION, CommandPayloadInfo()},
    {READ_NUM_PULSES_RECEIVED, CommandPayloadInfo()},
    {READ_IO_PORT_STATUS, CommandPayloadInfo()},
    {READ_MOTOR_SHAFT_ANGLE_ERROR, CommandPayloadInfo()},
    {READ_EN_PINS_STATUS, CommandPayloadInfo()},
    {READ_GO_BACK_TO_ZERO_STATUS_WHEN_POWER_ON, CommandPayloadInfo()},
    {RELEASE_MOTOR_SHAFT_LOCKED_PROTECTION_STATE, CommandPayloadInfo()},
    {READ_MOTOR_SHAFT_PROTECTION_STATE, CommandPayloadInfo()},
    {RESTORE_DEFAULT_PARAMETERS, CommandPayloadInfo()},
    {RESTART, CommandPayloadInfo()},
    {GO_HOME, CommandPayloadInfo()},
    {SET_CURRENT_AXIS_TO_ZERO, CommandPayloadInfo()},
    {EMERGENCY_STOP, CommandPayloadInfo()},
    {QUERY_MOTOR_STATUS, CommandPayloadInfo()},
    {ENABLE_MOTOR, CommandPayloadInfo()},
    // Commands with a single parameter as payload
    {SET_WORKING_CURRENT, CommandPayloadInfo(PayloadType::UINT16)},
    {SET_SUBDIVISIONS, CommandPayloadInfo(PayloadType::UINT8)},
    {SET_EN_PIN_CONFIG, CommandPayloadInfo(PayloadType::UINT8)},
    {SET_MOTOR_ROTATION_DIRECTION, CommandPayloadInfo(PayloadType::UINT8)},
    {SET_AUTO_TURN_OFF_SCREEN, CommandPayloadInfo(PayloadType::UINT8)},
    {SET_MOTOR_SHAFT_LOCKED_ROTOR_PROTECTION, CommandPayloadInfo(PayloadType::UINT8)},
    {SET_SUBDIVISION_INTERPOLATION, CommandPayloadInfo(PayloadType::UINT8)},
    {SET_CAN_BITRATE, CommandPayloadInfo(PayloadType::UINT8)},
    {SET_CAN_ID, CommandPayloadInfo(PayloadType::UINT16)},
    {SET_KEY_LOCK_ENABLE, CommandPayloadInfo(PayloadType::UINT8)},

    {SET_HOME, CommandPayloadInfo(PayloadType::UINT8, PayloadType::UINT8, PayloadType::UINT16, PayloadType::UINT8)},

    {RUN_MOTOR_ABSOLUTE_MOTION_BY_PULSES, CommandPayloadInfo(PayloadType::UINT16, PayloadType::UINT8, PayloadType::INT24)},
    {RUN_MOTOR_RELATIVE_MOTION_BY_PULSES, CommandPayloadInfo(PayloadType::UINT16, PayloadType::UINT8, PayloadType::INT24)},
    {RUN_MOTOR_ABSOLUTE_MOTION_BY_AXIS, CommandPayloadInfo(PayloadType::UINT16, PayloadType::UINT8, PayloadType::INT24)},
    {RUN_MOTOR_RELATIVE_MOTION_BY_AXIS, CommandPayloadInfo(PayloadType::UINT16, PayloadType::UINT8, PayloadType::INT24)}};

#endif // COMMAND_PAYLOAD_TYPEDEFS_HPP