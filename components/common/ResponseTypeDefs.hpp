#ifndef RESPONSE_TYPEDEFS_HPP
#define RESPONSE_TYPEDEFS_HPP

#include "../motor/Properties.hpp"
#include "MksEnums.hpp"
#include <any>
#include <array>
#include <esp_event.h>
#include <esp_log.h>
#include <map>
#include <memory>
#include <optional>
#include <type_traits>
#include <variant>

using ResponseProperties = std::array<std::any, 7>;
using PropertyNames = std::vector<std::string>;
struct ResponseInformation
{
    CommandPayloadInfo payload_info;
    PropertyNames property_names;

    ResponseInformation(CommandPayloadInfo type_info, std::vector<std::string> properties)
        : payload_info(type_info), property_names(properties) {}

    size_t get_num_properties() const { return property_names.size(); }
};

const std::map<CommandIds, ResponseInformation> g_response_payload_map = {
    {QUERY_MOTOR_STATUS, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), {"motor_status"})},
    {MOTOR_CALIBRATION, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), {"calibration_status"})},
    {EMERGENCY_STOP, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), {"emergency_stop_triggered"})},
    {READ_ENCODER_VALUE_CARRY, ResponseInformation(CommandPayloadInfo(PayloadType::INT32, PayloadType::UINT16), {"current_position", "encoder_value"})},
    {ENABLE_MOTOR, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), {"is_enabled"})},
    {GO_HOME, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), {"home_dir"})},
    {SET_CURRENT_AXIS_TO_ZERO, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), {"current_position"})},
    {RESTORE_DEFAULT_PARAMETERS, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), {"save_clean_state"})},
    {RELEASE_MOTOR_SHAFT_LOCKED_PROTECTION_STATE, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), {"motor_shaft_protection_status"})},
    {READ_MOTOR_SHAFT_PROTECTION_STATE, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), {"motor_shaft_protection_status"})},
    {READ_EN_PINS_STATUS, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), {"en_pin_config"})},
    {READ_IO_PORT_STATUS, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), {"group_id"})},
    {READ_GO_BACK_TO_ZERO_STATUS_WHEN_POWER_ON, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), {"home_trig"})},
    {SET_WORKING_CURRENT, ResponseInformation(CommandPayloadInfo(PayloadType::UINT16), {"working_current"})},
    {SET_SUBDIVISIONS, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), {"subdivisions"})},
    {SET_EN_PIN_CONFIG, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), {"en_pin_config"})},
    {SET_MOTOR_ROTATION_DIRECTION, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), {"motor_rotation_direction"})},
    {SET_AUTO_TURN_OFF_SCREEN, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), {"auto_turn_off_screen"})},
    {SET_MOTOR_SHAFT_LOCKED_ROTOR_PROTECTION, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), {"locked_rotor_protection"})},
    {SET_SUBDIVISION_INTERPOLATION, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), {"subdivision_interpolation"})},
    {SET_CAN_BITRATE, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), {"can_bitrate"})},
    {SET_CAN_ID, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), {"can_id"})},
    {SET_KEY_LOCK_ENABLE, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), {"key_lock_enabled"})},
    {SET_HOME, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), {"home_dir"})},
    {RUN_MOTOR_SPEED_MODE, ResponseInformation(CommandPayloadInfo(PayloadType::UINT32), {"current_speed"})},
    {RUN_MOTOR_ABSOLUTE_MOTION_BY_AXIS, ResponseInformation(CommandPayloadInfo(PayloadType::INT32), {"target_position"})},
    {RUN_MOTOR_RELATIVE_MOTION_BY_AXIS, ResponseInformation(CommandPayloadInfo(PayloadType::INT32), {"target_position"})},
    {RUN_MOTOR_RELATIVE_MOTION_BY_PULSES, ResponseInformation(CommandPayloadInfo(PayloadType::INT32), {"target_position"})},
    {RUN_MOTOR_ABSOLUTE_MOTION_BY_PULSES, ResponseInformation(CommandPayloadInfo(PayloadType::INT32), {"target_position"})},
    {READ_ENCODED_VALUE_ADDITION, ResponseInformation(CommandPayloadInfo(PayloadType::INT32), {"current_position"})},
    {READ_MOTOR_SPEED, ResponseInformation(CommandPayloadInfo(PayloadType::UINT32), {"current_speed"})},
    {READ_NUM_PULSES_RECEIVED, ResponseInformation(CommandPayloadInfo(PayloadType::INT32), {"target_position"})},
    {READ_MOTOR_SHAFT_ANGLE_ERROR, ResponseInformation(CommandPayloadInfo(PayloadType::INT32), {"current_position"})},
    {LAST_SEEN, ResponseInformation(CommandPayloadInfo(PayloadType::VOID), {"last_seen"})}

};
// {LAST_SEEN, ResponseInformation(CommandPayloadInfo(PayloadType::CHRONO), ResponseProperties{std::any(&MotorProperties::last_seen})} // Last seen time
// {QUERY_MOTOR_STATUS, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), ResponseProperties{std::any(&MotorProperties::motor_status)})},
// {MOTOR_CALIBRATION, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), ResponseProperties{std::any(&MotorProperties::calibration_status)})},
// {EMERGENCY_STOP, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), ResponseProperties{std::any(&MotorProperties::emergency_stop_triggered)})},
// {ENABLE_MOTOR, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), ResponseProperties{std::any(&MotorProperties::is_enabled)})},
// {GO_HOME, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), ResponseProperties{std::any(&MotorProperties::home_dir)})},
// {SET_CURRENT_AXIS_TO_ZERO, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), ResponseProperties{std::any(&MotorProperties::current_position)})},
// {RESTORE_DEFAULT_PARAMETERS, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), ResponseProperties{std::any(&MotorProperties::save_clean_state)})},
// {RELEASE_MOTOR_SHAFT_LOCKED_PROTECTION_STATE, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), ResponseProperties{std::any(&MotorProperties::motor_shaft_protection_status)})},
// {READ_MOTOR_SHAFT_PROTECTION_STATE, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), ResponseProperties{std::any(&MotorProperties::motor_shaft_protection_status)})},
// {READ_EN_PINS_STATUS, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), ResponseProperties{std::any(&MotorProperties::en_pin_config)})},
// {READ_IO_PORT_STATUS, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), ResponseProperties{std::any(&MotorProperties::group_id)})},
// {READ_GO_BACK_TO_ZERO_STATUS_WHEN_POWER_ON, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), ResponseProperties{std::any(&MotorProperties::home_trig)})},
// {SET_WORKING_CURRENT, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), ResponseProperties{std::any(&MotorProperties::working_current)})},
// {SET_SUBDIVISIONS, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), ResponseProperties{std::any(&MotorProperties::subdivisions)})},
// {SET_EN_PIN_CONFIG, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), ResponseProperties{std::any(&MotorProperties::en_pin_config)})},
// {SET_MOTOR_ROTATION_DIRECTION, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), ResponseProperties{std::any(&MotorProperties::motor_rotation_direction)})},
// {SET_AUTO_TURN_OFF_SCREEN, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), ResponseProperties{std::any(&MotorProperties::auto_turn_off_screen)})},
// {SET_MOTOR_SHAFT_LOCKED_ROTOR_PROTECTION, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), ResponseProperties{std::any(&MotorProperties::locked_rotor_protection)})},
// {SET_SUBDIVISION_INTERPOLATION, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), ResponseProperties{std::any(&MotorProperties::subdivision_interpolation)})},
// {SET_CAN_BITRATE, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), ResponseProperties{std::any(&MotorProperties::can_bitrate)})},
// {SET_CAN_ID, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), ResponseProperties{std::any(&MotorProperties::can_id)})},
// {SET_KEY_LOCK_ENABLE, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), ResponseProperties{std::any(&MotorProperties::key_lock_enabled)})},
// {SET_HOME, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), ResponseProperties{std::any(&MotorProperties::home_dir)})},

// {RUN_MOTOR_SPEED_MODE, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), ResponseProperties{std::any(&MotorProperties::current_speed)})},
// {RUN_MOTOR_ABSOLUTE_MOTION_BY_AXIS, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), ResponseProperties{std::any(&MotorProperties::run_motor_result)})},   // Speed, acceleration, and absolute axis (uint24)
// {RUN_MOTOR_RELATIVE_MOTION_BY_AXIS, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), ResponseProperties{std::any(&MotorProperties::run_motor_result)})},   // Speed, acceleration, and relative axis (uint24)
// {RUN_MOTOR_RELATIVE_MOTION_BY_PULSES, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), ResponseProperties{std::any(&MotorProperties::run_motor_result)})}, // Speed, acceleration, relative pulses (uint24)
// {RUN_MOTOR_ABSOLUTE_MOTION_BY_PULSES, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), ResponseProperties{std::any(&MotorProperties::run_motor_result)})}, // Speed, acceleration, absolute pulses (uint24)

// {READ_ENCODER_VALUE_CARRY, ResponseInformation(CommandPayloadInfo(PayloadType::INT32, PayloadType::UINT16), ResponseProperties{std::any(&MotorProperties::current_position)})}, // Carry (int32) and encoder value (uint16)
// {READ_ENCODED_VALUE_ADDITION, ResponseInformation(CommandPayloadInfo(PayloadType::INT48), ResponseProperties{std::any(&MotorProperties::current_position)})},                   // Encoder value (int48)
// {READ_MOTOR_SPEED, ResponseInformation(CommandPayloadInfo(PayloadType::INT16), ResponseProperties{std::any(&MotorProperties::current_speed)})},                                 // Speed (int16)
// {READ_NUM_PULSES_RECEIVED, ResponseInformation(CommandPayloadInfo(PayloadType::INT32), ResponseProperties{std::any(&MotorProperties::target_position)})},                       // Number of pulses (int32)
// {READ_MOTOR_SHAFT_ANGLE_ERROR, ResponseInformation(CommandPayloadInfo(PayloadType::INT32), ResponseProperties{std::any(&MotorProperties::current_position)})}                   // Shaft angle error (int32)

//     // Commands with standard 2-byte response (CommandId and Status)
//     {MOTOR_CALIBRATION, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), std::array<std::any, 7>{&MotorProperties::calibration_status})},              // Status only
//     {EMERGENCY_STOP, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), std::array<std::any, 7>{&MotorProperties::dummy})},                              // Status only
//     {QUERY_MOTOR_STATUS, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), std::array<std::any, 7>{&MotorProperties::motor_status})},                   // Status only
//     {ENABLE_MOTOR, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), std::array<std::any, 7>{&MotorProperties::dummy})},                                // Status only
//     {GO_HOME, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), std::array<std::any, 7>{&MotorProperties::dummy})},                                     // Status only
//     {SET_CURRENT_AXIS_TO_ZERO, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), std::array<std::any, 7>{&MotorProperties::dummy})},                    // Status only
//     {RESTORE_DEFAULT_PARAMETERS, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), std::array<std::any, 7>{&MotorProperties::dummy})},                  // Status only
//     {RELEASE_MOTOR_SHAFT_LOCKED_PROTECTION_STATE, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), std::array<std::any, 7>{&MotorProperties::dummy})}, // Status only
//     {READ_MOTOR_SHAFT_PROTECTION_STATE, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), std::array<std::any, 7>{&MotorProperties::dummy})},           // Status only
//     {READ_EN_PINS_STATUS, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), std::array<std::any, 7>{&MotorProperties::dummy})},                         // Status only
//     {READ_IO_PORT_STATUS, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), std::array<std::any, 7>{&MotorProperties::dummy})},                         // Status only
//     {READ_GO_BACK_TO_ZERO_STATUS_WHEN_POWER_ON, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), std::array<std::any, 7>{&MotorProperties::dummy})},   // Status only
//     {SET_WORKING_CURRENT, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), std::array<std::any, 7>{&MotorProperties::dummy})},                         // Status only
//     {SET_SUBDIVISIONS, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), std::array<std::any, 7>{&MotorProperties::dummy})},                            // Status only
//     {SET_EN_PIN_CONFIG, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), std::array<std::any, 7>{&MotorProperties::dummy})},                           // Status only
//     {SET_MOTOR_ROTATION_DIRECTION, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), std::array<std::any, 7>{&MotorProperties::dummy})},                // Status only
//     {SET_AUTO_TURN_OFF_SCREEN, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), std::array<std::any, 7>{&MotorProperties::dummy})},                    // Status only
//     {SET_MOTOR_SHAFT_LOCKED_ROTOR_PROTECTION, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), std::array<std::any, 7>{&MotorProperties::dummy})},     // Status only
//     {SET_SUBDIVISION_INTERPOLATION, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), std::array<std::any, 7>{&MotorProperties::dummy})},               // Status only
//     {SET_CAN_BITRATE, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), std::array<std::any, 7>{&MotorProperties::dummy})},                             // Status only
//     {SET_CAN_ID, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), std::array<std::any, 7>{&MotorProperties::dummy})},                                  // Status only
//     {SET_KEY_LOCK_ENABLE, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), std::array<std::any, 7>{&MotorProperties::dummy})},                         // Status only
//     {SET_HOME, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), std::array<std::any, 7>{&MotorProperties::dummy})},                                    // Status only
//     {RUN_MOTOR_SPEED_MODE, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), std::array<std::any, 7>{&MotorProperties::dummy})},                        // Status only
//     {RUN_MOTOR_ABSOLUTE_MOTION_BY_AXIS, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), std::array<std::any, 7>{&MotorProperties::dummy})},           // Speed, acceleration, and absolute axis (uint24)
//     {RUN_MOTOR_RELATIVE_MOTION_BY_AXIS, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), std::array<std::any, 7>{&MotorProperties::dummy})},           // Speed, acceleration, and relative axis (uint24)
//     {RUN_MOTOR_RELATIVE_MOTION_BY_PULSES, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), std::array<std::any, 7>{&MotorProperties::dummy})},         // Speed, acceleration, relative pulses (uint24)
//     {RUN_MOTOR_ABSOLUTE_MOTION_BY_PULSES, ResponseInformation(CommandPayloadInfo(PayloadType::UINT8), std::array<std::any, 7>{&MotorProperties::dummy})},         // Speed, acceleration, absolute pulses (uint24)

//     // Commands with more complex responses
//     {READ_ENCODER_VALUE_CARRY, ResponseInformation(CommandPayloadInfo(PayloadType::INT32, PayloadType::UINT16), std::array<std::any, 7>{&MotorProperties::dummy})}, // Carry (int32) and encoder value (uint16)
//     {READ_ENCODED_VALUE_ADDITION, ResponseInformation(CommandPayloadInfo(PayloadType::INT48), std::array<std::any, 7>{&MotorProperties::dummy})},                   // Encoder value (int48)
//     {READ_MOTOR_SPEED, ResponseInformation(CommandPayloadInfo(PayloadType::INT16), std::array<std::any, 7>{&MotorProperties::dummy})},                              // Speed (int16)
//     {READ_NUM_PULSES_RECEIVED, ResponseInformation(CommandPayloadInfo(PayloadType::INT32), std::array<std::any, 7>{&MotorProperties::dummy})},                      // Number of pulses (int32)
//     {READ_MOTOR_SHAFT_ANGLE_ERROR, ResponseInformation(CommandPayloadInfo(PayloadType::INT32), std::array<std::any, 7>{&MotorProperties::dummy})},                  // Shaft angle error (int32)

//     {LAST_SEEN, ResponseInformation(CommandPayloadInfo(PayloadType::CHRONO), std::array<std::any, 7>{&MotorProperties::last_seen})}, // Last seen time
// };

#endif // RESPONSE_TYPEDEFS_HPP