#include "Properties.hpp"

extern std::map<std::string, PropertyMetadata> property_metadata_map = {
    {"motor_status", {"motor_status", PayloadType::UINT8, offsetof(MotorProperties, motor_status), get_payload_type_size(PayloadType::UINT8), true}},
    {"calibration_status", {"calibration_status", PayloadType::UINT8, offsetof(MotorProperties, calibration_status), get_payload_type_size(PayloadType::UINT8), true}},
    {"current_speed", {"current_speed", PayloadType::UINT32, offsetof(MotorProperties, current_speed), get_payload_type_size(PayloadType::UINT32), false}},
    {"current_position", {"current_position", PayloadType::INT32, offsetof(MotorProperties, current_position), get_payload_type_size(PayloadType::INT32), false}},
    {"target_position", {"target_position", PayloadType::INT32, offsetof(MotorProperties, target_position), get_payload_type_size(PayloadType::INT32), false}},
    {"can_id", {"can_id", PayloadType::UINT8, offsetof(MotorProperties, can_id), get_payload_type_size(PayloadType::UINT8), false}},
    {"can_bitrate", {"can_bitrate", PayloadType::UINT8, offsetof(MotorProperties, can_bitrate), get_payload_type_size(PayloadType::UINT8), true}},
    {"group_id", {"group_id", PayloadType::UINT8, offsetof(MotorProperties, group_id), get_payload_type_size(PayloadType::UINT8), false}},
    {"working_current", {"working_current", PayloadType::UINT16, offsetof(MotorProperties, working_current), get_payload_type_size(PayloadType::UINT16), false}},
    {"holding_current", {"holding_current", PayloadType::UINT16, offsetof(MotorProperties, holding_current), get_payload_type_size(PayloadType::UINT16), false}},
    {"motor_rotation_direction", {"motor_rotation_direction", PayloadType::UINT8, offsetof(MotorProperties, motor_rotation_direction), get_payload_type_size(PayloadType::UINT8), true}},
    {"subdivisions", {"subdivisions", PayloadType::UINT8, offsetof(MotorProperties, subdivisions), get_payload_type_size(PayloadType::UINT8), false}},
    {"en_pin_config", {"en_pin_config", PayloadType::UINT8, offsetof(MotorProperties, en_pin_config), get_payload_type_size(PayloadType::UINT8), true}},
    {"key_lock_enabled", {"key_lock_enabled", PayloadType::UINT8, offsetof(MotorProperties, key_lock_enabled), get_payload_type_size(PayloadType::UINT8), true}},
    {"auto_turn_off_screen", {"auto_turn_off_screen", PayloadType::UINT8, offsetof(MotorProperties, auto_turn_off_screen), get_payload_type_size(PayloadType::UINT8), true}},
    {"locked_rotor_protection", {"locked_rotor_protection", PayloadType::UINT8, offsetof(MotorProperties, locked_rotor_protection), get_payload_type_size(PayloadType::UINT8), true}},
    {"subdivision_interpolation", {"subdivision_interpolation", PayloadType::UINT8, offsetof(MotorProperties, subdivision_interpolation), get_payload_type_size(PayloadType::UINT8), true}},
    {"home_trig", {"home_trig", PayloadType::UINT8, offsetof(MotorProperties, home_trig), get_payload_type_size(PayloadType::UINT8), true}},
    {"home_dir", {"home_dir", PayloadType::UINT8, offsetof(MotorProperties, home_dir), get_payload_type_size(PayloadType::UINT8), true}},
    {"home_speed", {"home_speed", PayloadType::UINT16, offsetof(MotorProperties, home_speed), get_payload_type_size(PayloadType::UINT16), false}},
    {"end_limit", {"end_limit", PayloadType::UINT8, offsetof(MotorProperties, end_limit), get_payload_type_size(PayloadType::UINT8), true}},
    {"emergency_stop_triggered", {"emergency_stop_triggered", PayloadType::UINT8, offsetof(MotorProperties, emergency_stop_triggered), get_payload_type_size(PayloadType::UINT8), true}},
    {"is_enabled", {"is_enabled", PayloadType::UINT8, offsetof(MotorProperties, is_enabled), get_payload_type_size(PayloadType::UINT8), true}},
    {"mode0_status", {"mode0_status", PayloadType::UINT8, offsetof(MotorProperties, mode0_status), get_payload_type_size(PayloadType::UINT8), true}},
    {"motor_shaft_protection_status", {"motor_shaft_protection_status", PayloadType::UINT8, offsetof(MotorProperties, motor_shaft_protection_status), get_payload_type_size(PayloadType::UINT8), true}},
    {"save_clean_state", {"save_clean_state", PayloadType::UINT8, offsetof(MotorProperties, save_clean_state), get_payload_type_size(PayloadType::UINT8), true}},
    {"last_seen", {"last_seen", PayloadType::VOID, offsetof(MotorProperties, last_seen), sizeof(std::chrono::system_clock::time_point), false}},
    {"dummy", {"dummy", PayloadType::VOID, offsetof(MotorProperties, dummy), sizeof(std::any), false}}
    // Add entries for all properties
};