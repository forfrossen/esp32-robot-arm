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

struct PropertyChangeEventData
{
    std::string property_name;
    PayloadType type;
    union
    {
        uint8_t uint8_value;
        uint16_t uint16_value;
        int16_t int16_value;
        int32_t int32_value;
        uint32_t uint32_value;
        uint64_t uint48_value;
        std::chrono::system_clock::time_point chrono_value;
    } value;
};

struct ResponsePropertyMetadata
{
    std::string name;
    PayloadType type;
    size_t offset;
    size_t size;

    bool is_enum;
    bool (*setter)(void *, uint64_t);
};

struct MotorProperties
{
    MotorStatus motor_status;
    int32_t current_position;
    int32_t target_position;
    int32_t full_turns_count;
    uint16_t current_turn_position;

    // Command Response Properties
    RunMotorResult run_motor_result;
    CalibrationResult calibration_status;
    float current_speed;

    // CAN Related Properties
    uint8_t can_id;
    uint8_t can_bitrate;
    uint8_t group_id;

    // Settings of the Motor
    uint16_t working_current;
    uint16_t holding_current;
    Direction motor_rotation_direction;
    uint8_t subdivisions;
    EnPinEnable en_pin_config;
    SuccessStatus set_en_pin_config_status;
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
    std::any dummy;
};

extern bool set_uint8_property(void *prop_ptr, uint64_t raw_value);

extern bool set_uint16_property(void *prop_ptr, uint64_t raw_value);

extern bool set_uint32_property(void *prop_ptr, uint64_t raw_value);

extern bool set_int16_property(void *prop_ptr, uint64_t raw_value);

extern bool set_int32_property(void *prop_ptr, uint64_t raw_value);

extern bool set_uint24_property(void *prop_ptr, uint64_t raw_value);

extern bool set_int48_property(void *prop_ptr, uint64_t raw_value);

template <typename EnumType>
extern bool set_enum_uint8_property(void *prop_ptr, uint64_t raw_value);

extern std::map<std::string, ResponsePropertyMetadata> response_property_metadata_map;

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

// // Mapping motor_command_id_t to uint16_t properties
// const std::map<motor_command_id_t, CommandPropertyMapping<uint16_t>> uint16_properties_map = {
//     {SET_WORKING_CURRENT, &MotorProperties::working_current},
//     {SET_HOLDING_CURRENT, &MotorProperties::holding_current},
//     {SET_HOME, &MotorProperties::home_speed},
// };

// // Mapping motor_command_id_t to uint8_t properties
// const std::map<motor_command_id_t, CommandPropertyMapping<uint8_t>> uint8_properties_map = {
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

// // Mapping motor_command_id_t to bool properties
// const std::map<motor_command_id_t, CommandPropertyMapping<bool>> bool_properties_map = {
//     {EMERGENCY_STOP, &MotorProperties::emergency_stop_triggered},
//     {ENABLE_MOTOR, &MotorProperties::is_enabled},
//     {SET_KEY_LOCK_ENABLE, &MotorProperties::key_lock_enabled},
//     {SET_AUTO_TURN_OFF_SCREEN, &MotorProperties::auto_turn_off_screen},
//     {SET_MOTOR_SHAFT_LOCKED_ROTOR_PROTECTION, &MotorProperties::locked_rotor_protection},
//     {SET_SUBDIVISION_INTERPOLATION, &MotorProperties::subdivision_interpolation},
//     {RUN_MOTOR_SPEED_MODE, &MotorProperties::is_moving},
// };

// // Mapping motor_command_id_t to float properties
// const std::map<motor_command_id_t, CommandPropertyMapping<float>> float_properties_map = {
//     {READ_MOTOR_SPEED, &MotorProperties::current_speed},
// };

// // Mapping motor_command_id_t to int32_t properties
// const std::map<motor_command_id_t, CommandPropertyMapping<int32_t>> int32_properties_map = {
//     {READ_MOTOR_POSITION, &MotorProperties::current_position},
//     {READ_MOTOR_POSITION, &MotorProperties::target_position},
// };
