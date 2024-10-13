#ifndef MOTOR_PROPERTIES_HPP
#define MOTOR_PROPERTIES_HPP

#include "../magic_enum/include/magic_enum/magic_enum.hpp"
#include <cstdint>
#include <iostream>
#include <map>

struct MotorProperties
{
    // Working current in mA (0-3000 mA)
    uint16_t working_current = 0;

    // Holding current in mA
    uint16_t holding_current = 0;

    // Motor rotation direction (0 = CW, 1 = CCW)
    uint8_t motor_rotation_direction = 0;

    // Subdivisions (usually a value like 16, 32, 64)
    uint8_t subdivisions = 0;

    // Enable pin configuration (0 = disabled, 1 = enabled)
    uint8_t en_pin_config = 0;

    // Key lock enable (0 = disabled, 1 = enabled)
    bool key_lock_enabled = false;

    // Auto turn off screen (0 = disabled, 1 = enabled)
    bool auto_turn_off_screen = false;

    // Locked rotor protection (0 = disabled, 1 = enabled)
    bool locked_rotor_protection = false;

    // Subdivision interpolation (0 = disabled, 1 = enabled)
    bool subdivision_interpolation = false;

    // CAN bus ID (default 0x01, 1-127)
    uint8_t can_id = 1;

    // CAN bus bitrate (e.g., 125, 250, 500 kbps)
    uint8_t can_bitrate = 0;

    // Group ID for the motor (used in CAN communication)
    uint8_t group_id = 0;

    // Home trigger level (0 = low, 1 = high)
    uint8_t home_trig = 0;

    // Home direction (0 = CW, 1 = CCW)
    uint8_t home_dir = 0;

    // Home speed (0-3000 RPM)
    uint16_t home_speed = 0;

    // End limit switch (0 = disabled, 1 = enabled)
    uint8_t end_limit = 0;

    // Emergency stop triggered (0 = not triggered, 1 = triggered)
    bool emergency_stop_triggered = false;

    // Motor enabled (0 = disabled, 1 = enabled)
    bool is_enabled = false;

    // Motor's current position (read from encoder)
    int32_t current_position = 0;

    // Target position for motor movement
    int32_t target_position = 0;

    // Motor's current speed in RPM
    float current_speed = 0.0f;

    // Whether the motor is currently moving (true = moving, false = stationary)
    bool is_moving = false;

    // Mode0 configuration (depending on the motor's specific mode, could be a uint8_t)
    uint8_t mode0 = 0;
};

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

#endif // MOTOR_PROPERTIES_HPP