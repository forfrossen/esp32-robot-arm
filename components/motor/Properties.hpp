#ifndef MOTOR_PROPERTIES_HPP
#define MOTOR_PROPERTIES_HPP

#include "../magic_enum/include/magic_enum/magic_enum.hpp"

#include "MksEnums.hpp"

#include <any>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cxxabi.h>
#include <functional>
#include <iostream>
#include <map>
#include <typeinfo>

struct PropertyMetadata
{
    std::string name;
    PayloadType type;
    size_t offset; // Offset within MotorProperties
    size_t size;   // Size of the property type
    bool is_enum;  // Indicates if the property is an enum
};

struct MotorProperties
{
    MotorStatus motor_status;
    CalibrationResult calibration_status;
    float current_speed;
    int32_t current_position;
    int32_t target_position;
    uint8_t can_id;
    uint8_t can_bitrate;
    uint8_t group_id;
    uint16_t working_current;
    uint16_t holding_current;
    Direction motor_rotation_direction;
    uint8_t subdivisions;
    EnPinEnable en_pin_config;
    bool key_lock_enabled;
    bool auto_turn_off_screen;
    bool locked_rotor_protection;
    uint8_t subdivision_interpolation;
    uint8_t home_trig;
    uint8_t home_dir;
    uint16_t home_speed;
    uint8_t end_limit;
    bool emergency_stop_triggered;
    bool is_enabled;
    Mode0 mode0_status;
    MotorShaftProtectionStatus motor_shaft_protection_status;
    SaveCleanState save_clean_state;
    std::chrono::system_clock::time_point last_seen;
    std::any dummy;
};

extern std::map<std::string, PropertyMetadata> property_metadata_map;

#endif // MOTOR_PROPERTIES_HPP

// struct MotorProperties
// {
//     MotorProperty<MotorStatus> motor_status = MotorProperty<MotorStatus>(MotorStatus::UNKNOWN, PayloadType::UINT8);

//     MotorProperty<RunMotorResult> run_motor_result = MotorProperty<RunMotorResult>(RunMotorResult::UNKNOWN, PayloadType::UINT8);
//     MotorProperty<float> current_speed = MotorProperty<float>(0.0f, PayloadType::UINT32);
//     MotorProperty<int32_t> current_position = MotorProperty<int32_t>(0, PayloadType::INT32);
//     MotorProperty<int32_t> target_position = MotorProperty<int32_t>(0, PayloadType::INT32);

//     MotorProperty<uint8_t> can_id = MotorProperty<uint8_t>(1, PayloadType::UINT8);
//     MotorProperty<CanBitrate> can_bitrate = MotorProperty<CanBitrate>(CanBitrate::Rate125K, PayloadType::UINT8);
//     MotorProperty<uint8_t> group_id = MotorProperty<uint8_t>(0, PayloadType::UINT8);

//     MotorProperty<uint16_t> working_current = MotorProperty<uint16_t>(0, PayloadType::UINT16);
//     MotorProperty<uint16_t> holding_current = MotorProperty<uint16_t>(0, PayloadType::UINT16);
//     MotorProperty<Direction> motor_rotation_direction = MotorProperty<Direction>(Direction::CW, PayloadType::UINT8);
//     MotorProperty<uint8_t> subdivisions = MotorProperty<uint8_t>(0, PayloadType::UINT8);
//     MotorProperty<EnPinEnable> en_pin_config = MotorProperty<EnPinEnable>(EnPinEnable::ActiveLow, PayloadType::UINT8);
//     MotorProperty<Enable> key_lock_enabled = MotorProperty<Enable>(Enable::Disable, PayloadType::UINT8);
//     MotorProperty<Enable> auto_turn_off_screen = MotorProperty<Enable>(Enable::Disable, PayloadType::UINT8);
//     MotorProperty<Enable> locked_rotor_protection = MotorProperty<Enable>(Enable::Disable, PayloadType::UINT8);
//     MotorProperty<Enable> subdivision_interpolation = MotorProperty<Enable>(Enable::Disable, PayloadType::UINT8);
//     MotorProperty<EndStopLevel> home_trig = MotorProperty<EndStopLevel>(EndStopLevel::Low, PayloadType::UINT8);
//     MotorProperty<Direction> home_dir = MotorProperty<Direction>(Direction::CW, PayloadType::UINT8);
//     MotorProperty<uint16_t> home_speed = MotorProperty<uint16_t>(0, PayloadType::UINT16);
//     MotorProperty<EndStopLevel> end_limit = MotorProperty<EndStopLevel>(EndStopLevel::Low, PayloadType::UINT8);
//     MotorProperty<Enable> emergency_stop_triggered = MotorProperty<Enable>(Enable::Disable, PayloadType::UINT8);
//     MotorProperty<EnableStatus> is_enabled = MotorProperty<EnableStatus>(EnableStatus::Disabled, PayloadType::UINT8);
//     MotorProperty<Mode0> mode0_status = MotorProperty<Mode0>(Mode0::Disable, PayloadType::UINT8);
//     MotorProperty<CalibrationResult> calibration_status = MotorProperty<CalibrationResult>(CalibrationResult::Calibrating, PayloadType::UINT8);

//     MotorProperty<MotorShaftProtectionStatus> motor_shaft_protection_status = MotorProperty<MotorShaftProtectionStatus>(MotorShaftProtectionStatus::UNKNOWN, PayloadType::UINT8);
//     MotorProperty<SaveCleanState> save_clean_state = MotorProperty<SaveCleanState>(SaveCleanState::UNKNOWN, PayloadType::UINT8);

//     Property<std::chrono::system_clock::time_point> last_seen = Property(std::chrono::system_clock::time_point{});
//     Property<std::any> dummy = Property(std::any{});
// };

// MotorProperty working_current = MotorProperty(uint16_t(0), PayloadType::UINT16);
// MotorProperty holding_current = MotorProperty(uint16_t(0), PayloadType::UINT16);
// MotorProperty motor_rotation_direction = MotorProperty(Direction::CW, PayloadType::UINT8);
// MotorProperty subdivisions = MotorProperty(uint8_t(0), PayloadType::UINT8);
// MotorProperty en_pin_config = MotorProperty(EnPinEnable::ActiveLow, PayloadType::UINT8);
// MotorProperty key_lock_enabled = MotorProperty(Enable::Disable, PayloadType::UINT8);
// MotorProperty auto_turn_off_screen = MotorProperty(Enable::Disable, PayloadType::UINT8);
// MotorProperty locked_rotor_protection = MotorProperty(Enable::Disable, PayloadType::UINT8);
// MotorProperty subdivision_interpolation = MotorProperty(Enable::Disable, PayloadType::UINT8);
// MotorProperty can_id = MotorProperty(uint8_t(1), PayloadType::UINT8);
// MotorProperty can_bitrate = MotorProperty(CanBitrate::Rate125K, PayloadType::UINT8);
// MotorProperty group_id = MotorProperty(uint8_t(0), PayloadType::UINT8);
// MotorProperty home_trig = MotorProperty(EndStopLevel::Low, PayloadType::UINT8);
// MotorProperty home_dir = MotorProperty(Direction::CW, PayloadType::UINT8);
// MotorProperty home_speed = MotorProperty(uint16_t(0), PayloadType::UINT16);
// MotorProperty end_limit = MotorProperty(EndStopLevel::Low, PayloadType::UINT8);
// MotorProperty emergency_stop_triggered = MotorProperty(Enable::Disable, PayloadType::UINT8);
// MotorProperty is_enabled = MotorProperty(EnableStatus::Disabled, PayloadType::UINT8);
// MotorProperty current_position = MotorProperty(int32_t(0), PayloadType::INT32);
// MotorProperty target_position = MotorProperty(int32_t(0), PayloadType::INT32);
// MotorProperty current_speed = MotorProperty(float(0.0f), PayloadType::UINT32);
// MotorProperty is_moving = MotorProperty(EnableStatus::Disabled, PayloadType::UINT8);
// MotorProperty mode0_status = MotorProperty(Mode0::Disable, PayloadType::UINT8);
// MotorProperty calibration_status = MotorProperty(CalibrationResult::Calibrating, PayloadType::UINT8);
// MotorProperty motor_status = MotorProperty(MotorStatus::UNKNOWN, PayloadType::UINT8);
// MotorProperty run_motor_result = MotorProperty(RunMotorResult::UNKNOWN, PayloadType::UINT8);
// MotorProperty motor_shaft_protection_status = MotorProperty(MotorShaftProtectionStatus::UNKNOWN, PayloadType::UINT8);
// MotorProperty save_clean_state = MotorProperty(SaveCleanState::UNKNOWN, PayloadType::UINT8);
// Property last_seen = Property(std::chrono::system_clock::time_point{});
// Property dummy = MotorProperty(uint8_t(0), PayloadType::UINT8);
// // Working current in mA (0-3000 mA)
// uint16_t working_current = 0;

// // Holding current in mA
// uint16_t holding_current = 0;

// // Motor rotation direction (0 = CW, 1 = CCW)
// uint8_t motor_rotation_direction = 0;

// // Subdivisions (usually a value like 16, 32, 64)
// uint8_t subdivisions = 0;

// // Enable pin configuration (0 = disabled, 1 = enabled)
// uint8_t en_pin_config = 0;

// // Key lock enable (0 = disabled, 1 = enabled)
// bool key_lock_enabled = false;

// // Auto turn off screen (0 = disabled, 1 = enabled)
// bool auto_turn_off_screen = false;

// // Locked rotor protection (0 = disabled, 1 = enabled)
// bool locked_rotor_protection = false;

// // Subdivision interpolation (0 = disabled, 1 = enabled)
// bool subdivision_interpolation = false;

// // CAN bus ID (default 0x01, 1-127)
// uint8_t can_id = 1;

// // CAN bus bitrate (e.g., 125, 250, 500 kbps)
// uint8_t can_bitrate = 0;

// // Group ID for the motor (used in CAN communication)
// uint8_t group_id = 0;

// // Home trigger level (0 = low, 1 = high)
// uint8_t home_trig = 0;

// // Home direction (0 = CW, 1 = CCW)
// uint8_t home_dir = 0;

// // Home speed (0-3000 RPM)
// uint16_t home_speed = 0;

// // End limit switch (0 = disabled, 1 = enabled)
// uint8_t end_limit = 0;

// // Emergency stop triggered (0 = not triggered, 1 = triggered)
// bool emergency_stop_triggered = false;

// // Motor enabled (0 = disabled, 1 = enabled)
// bool is_enabled = false;

// // Motor's current position (read from encoder)
// int32_t current_position = 0;

// // Target position for motor movement
// int32_t target_position = 0;

// // Motor's current speed in RPM
// float current_speed = 0.0f;

// // Whether the motor is currently moving (true = moving, false = stationary)
// bool is_moving = false;

// // Mode0 configuration (depending on the motor's specific mode, could be a uint8_t)
// uint8_t mode0 = 0;

// std::chrono::system_clock::time_point last_seen;

// bool calibration_status = false;

// std::any dummy;

// template <typename T>
// struct CommandPropertyMapping
// {
//     T MotorProperties::*property;
// };

// // Mapping CommandIds to uint16_t properties
// const std::map<CommandIds, CommandPropertyMapping<uint16_t>> uint16_properties_map = {
//     {SET_WORKING_CURRENT, &MotorProperties::working_current},
//     {SET_HOLDING_CURRENT, &MotorProperties::holding_current},
//     {SET_HOME, &MotorProperties::home_speed},
// };

// // Mapping CommandIds to uint8_t properties
// const std::map<CommandIds, CommandPropertyMapping<uint8_t>> uint8_properties_map = {
//     {SET_MOTOR_ROTATION_DIRECTION, &MotorProperties::motor_rotation_direction},
//     {SET_SUBDIVISIONS, &MotorProperties::subdivisions},
//     {SET_CAN_ID, &MotorProperties::can_id},
//     {SET_CAN_BITRATE, &MotorProperties::can_bitrate},
//     {SET_GROUP_ID, &MotorProperties::group_id},
//     {SET_HOME, &MotorProperties::home_trig},
//     {SET_HOME, &MotorProperties::home_dir},
//     {SET_HOME, &MotorProperties::end_limit},
//     {SET_MODE0, &MotorProperties::mode0},
// };

// // Mapping CommandIds to bool properties
// const std::map<CommandIds, CommandPropertyMapping<bool>> bool_properties_map = {
//     {EMERGENCY_STOP, &MotorProperties::emergency_stop_triggered},
//     {ENABLE_MOTOR, &MotorProperties::is_enabled},
//     {SET_KEY_LOCK_ENABLE, &MotorProperties::key_lock_enabled},
//     {SET_AUTO_TURN_OFF_SCREEN, &MotorProperties::auto_turn_off_screen},
//     {SET_MOTOR_SHAFT_LOCKED_ROTOR_PROTECTION, &MotorProperties::locked_rotor_protection},
//     {SET_SUBDIVISION_INTERPOLATION, &MotorProperties::subdivision_interpolation},
//     {RUN_MOTOR_SPEED_MODE, &MotorProperties::is_moving},
// };

// // Mapping CommandIds to float properties
// const std::map<CommandIds, CommandPropertyMapping<float>> float_properties_map = {
//     {READ_MOTOR_SPEED, &MotorProperties::current_speed},
// };

// // Mapping CommandIds to int32_t properties
// const std::map<CommandIds, CommandPropertyMapping<int32_t>> int32_properties_map = {
//     {READ_MOTOR_POSITION, &MotorProperties::current_position},
//     {READ_MOTOR_POSITION, &MotorProperties::target_position},
// };
