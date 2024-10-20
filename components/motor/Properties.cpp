#include "Properties.hpp"

#define DEFINE_SET_PROPERTY_FUNCTION(type, func_name, mask)        \
    bool func_name(void *prop_ptr, uint64_t raw_value)             \
    {                                                              \
        auto *prop_typed_ptr = reinterpret_cast<type *>(prop_ptr); \
        type new_value = static_cast<type>(raw_value & mask);      \
        if (*prop_typed_ptr != new_value)                          \
        {                                                          \
            *prop_typed_ptr = new_value;                           \
            return true;                                           \
        }                                                          \
        return false;                                              \
    }

// Special case for signed integers with sign extension
#define DEFINE_SET_SIGNED_PROPERTY_FUNCTION(type, func_name, mask, sign_bit) \
    bool func_name(void *prop_ptr, uint64_t raw_value)                       \
    {                                                                        \
        auto *prop_typed_ptr = reinterpret_cast<type *>(prop_ptr);           \
        type new_value = static_cast<type>(raw_value & mask);                \
        if (new_value & sign_bit)                                            \
        {                                                                    \
            new_value |= ~mask;                                              \
        }                                                                    \
        if (*prop_typed_ptr != new_value)                                    \
        {                                                                    \
            *prop_typed_ptr = new_value;                                     \
            return true;                                                     \
        }                                                                    \
        return false;                                                        \
    }

std::map<std::string, PropertyMetadata> property_metadata_map = {
    {"motor_status", {"motor_status", PayloadType::UINT8, offsetof(MotorProperties, motor_status), sizeof(MotorStatus), true, &set_enum_uint8_property<MotorStatus>}},
    {"run_motor_result", {"run_motor_result", PayloadType::UINT8, offsetof(MotorProperties, run_motor_result), sizeof(RunMotorResult), true, &set_enum_uint8_property<RunMotorResult>}},
    {"calibration_status", {"calibration_status", PayloadType::UINT8, offsetof(MotorProperties, calibration_status), sizeof(CalibrationResult), true, &set_enum_uint8_property<CalibrationResult>}},
    {"current_speed", {"current_speed", PayloadType::INT16, offsetof(MotorProperties, current_speed), sizeof(int16_t), false, &set_int16_property}},
    {"current_position", {"current_position", PayloadType::INT32, offsetof(MotorProperties, current_position), sizeof(int32_t), false, &set_int32_property}},
    {"calibration_status", {"calibration_status", PayloadType::UINT8, offsetof(MotorProperties, calibration_status), sizeof(uint8_t), false, &set_uint8_property}},
    {"current_speed", {"current_speed", PayloadType::UINT32, offsetof(MotorProperties, current_speed), sizeof(uint32_t), false, &set_uint32_property}},
    {"current_position", {"current_position", PayloadType::INT32, offsetof(MotorProperties, current_position), sizeof(int32_t), false, &set_int32_property}},
    {"target_position", {"target_position", PayloadType::INT32, offsetof(MotorProperties, target_position), sizeof(int32_t), false, &set_int32_property}},
    {"can_id", {"can_id", PayloadType::UINT8, offsetof(MotorProperties, can_id), sizeof(uint8_t), false, &set_uint8_property}},
    {"can_bitrate", {"can_bitrate", PayloadType::UINT8, offsetof(MotorProperties, can_bitrate), sizeof(uint8_t), false, &set_uint8_property}},
    {"group_id", {"group_id", PayloadType::UINT8, offsetof(MotorProperties, group_id), sizeof(uint8_t), false, &set_uint8_property}},
    {"working_current", {"working_current", PayloadType::UINT16, offsetof(MotorProperties, working_current), sizeof(uint16_t), false, &set_uint16_property}},
    {"holding_current", {"holding_current", PayloadType::UINT16, offsetof(MotorProperties, holding_current), sizeof(uint16_t), false, &set_uint16_property}},
    {"motor_rotation_direction", {"motor_rotation_direction", PayloadType::UINT8, offsetof(MotorProperties, motor_rotation_direction), sizeof(uint8_t), false, &set_uint8_property}},
    {"subdivisions", {"subdivisions", PayloadType::UINT8, offsetof(MotorProperties, subdivisions), sizeof(uint8_t), false, &set_uint8_property}},
    {"en_pin_config", {"en_pin_config", PayloadType::UINT8, offsetof(MotorProperties, en_pin_config), sizeof(uint8_t), false, &set_uint8_property}},
    {"key_lock_enabled", {"key_lock_enabled", PayloadType::UINT8, offsetof(MotorProperties, key_lock_enabled), sizeof(uint8_t), false, &set_uint8_property}},
    {"auto_turn_off_screen", {"auto_turn_off_screen", PayloadType::UINT8, offsetof(MotorProperties, auto_turn_off_screen), sizeof(uint8_t), false, &set_uint8_property}},
    {"locked_rotor_protection", {"locked_rotor_protection", PayloadType::UINT8, offsetof(MotorProperties, locked_rotor_protection), sizeof(uint8_t), false, &set_uint8_property}},
    {"subdivision_interpolation", {"subdivision_interpolation", PayloadType::UINT8, offsetof(MotorProperties, subdivision_interpolation), sizeof(uint8_t), false, &set_uint8_property}},
    {"home_trig", {"home_trig", PayloadType::UINT8, offsetof(MotorProperties, home_trig), sizeof(uint8_t), false, &set_uint8_property}},
    {"home_dir", {"home_dir", PayloadType::UINT8, offsetof(MotorProperties, home_dir), sizeof(uint8_t), false, &set_uint8_property}},
    {"home_speed", {"home_speed", PayloadType::UINT16, offsetof(MotorProperties, home_speed), sizeof(uint16_t), false, &set_uint16_property}},
    {"end_limit", {"end_limit", PayloadType::UINT8, offsetof(MotorProperties, end_limit), sizeof(uint8_t), false, &set_uint8_property}},
    {"emergency_stop_triggered", {"emergency_stop_triggered", PayloadType::UINT8, offsetof(MotorProperties, emergency_stop_triggered), sizeof(uint8_t), false, &set_uint8_property}},
    {"is_enabled", {"is_enabled", PayloadType::UINT8, offsetof(MotorProperties, is_enabled), sizeof(uint8_t), false, &set_uint8_property}},
    {"mode0_status", {"mode0_status", PayloadType::UINT8, offsetof(MotorProperties, mode0_status), sizeof(uint8_t), false, &set_uint8_property}},
    {"motor_shaft_protection_status", {"motor_shaft_protection_status", PayloadType::UINT8, offsetof(MotorProperties, motor_shaft_protection_status), sizeof(uint8_t), false, &set_uint8_property}},
    {"save_clean_state", {"save_clean_state", PayloadType::UINT8, offsetof(MotorProperties, save_clean_state), sizeof(uint8_t), false, &set_uint8_property}},
    {"last_seen", {"last_seen", PayloadType::VOID, offsetof(MotorProperties, last_seen), sizeof(std::chrono::system_clock::time_point), false, &set_uint8_property}},
    {"dummy", {"dummy", PayloadType::VOID, offsetof(MotorProperties, dummy), sizeof(std::any), false, &set_uint8_property}}};

// Define the property setter functions using the macro
DEFINE_SET_PROPERTY_FUNCTION(uint8_t, set_uint8_property, 0xFF)
DEFINE_SET_PROPERTY_FUNCTION(uint16_t, set_uint16_property, 0xFFFF)
DEFINE_SET_PROPERTY_FUNCTION(uint32_t, set_uint32_property, 0xFFFFFFFF)
DEFINE_SET_PROPERTY_FUNCTION(uint32_t, set_uint24_property, 0xFFFFFF)
DEFINE_SET_SIGNED_PROPERTY_FUNCTION(int16_t, set_int16_property, 0xFFFF, 0x8000)
DEFINE_SET_SIGNED_PROPERTY_FUNCTION(int32_t, set_int32_property, 0xFFFFFFFF, 0x80000000)
DEFINE_SET_SIGNED_PROPERTY_FUNCTION(int64_t, set_int48_property, 0xFFFFFFFFFFFF, 0x800000000000)

// bool set_uint8_property(void *prop_ptr, uint64_t raw_value)
// {
//     auto *prop_typed_ptr = reinterpret_cast<uint8_t *>(prop_ptr);
//     uint8_t new_value = static_cast<uint8_t>(raw_value & 0xFF);
//     if (*prop_typed_ptr != new_value)
//     {
//         *prop_typed_ptr = new_value;
//         return true;
//     }
//     return false;
// }

// bool set_uint16_property(void *prop_ptr, uint64_t raw_value)
// {
//     auto *prop_typed_ptr = reinterpret_cast<uint16_t *>(prop_ptr);
//     uint16_t new_value = static_cast<uint16_t>(raw_value & 0xFFFF);
//     if (*prop_typed_ptr != new_value)
//     {
//         *prop_typed_ptr = new_value;
//         return true;
//     }
//     return false;
// }

// bool set_uint32_property(void *prop_ptr, uint64_t raw_value)
// {
//     auto *prop_typed_ptr = reinterpret_cast<uint32_t *>(prop_ptr);
//     uint32_t new_value = static_cast<uint32_t>(raw_value & 0xFFFFFFFF);
//     if (*prop_typed_ptr != new_value)
//     {
//         *prop_typed_ptr = new_value;
//         return true;
//     }
//     return false;
// }

// bool set_int16_property(void *prop_ptr, uint64_t raw_value)
// {
//     auto *prop_typed_ptr = reinterpret_cast<int16_t *>(prop_ptr);
//     int16_t new_value = static_cast<int16_t>(raw_value & 0xFFFF);
//     // Sign extension
//     if (new_value & 0x8000)
//     {
//         new_value |= ~0xFFFF;
//     }
//     if (*prop_typed_ptr != new_value)
//     {
//         *prop_typed_ptr = new_value;
//         return true;
//     }
//     return false;
// }

// bool set_int32_property(void *prop_ptr, uint64_t raw_value)
// {
//     auto *prop_typed_ptr = reinterpret_cast<int32_t *>(prop_ptr);
//     int32_t new_value = static_cast<int32_t>(raw_value & 0xFFFFFFFF);
//     // Sign extension
//     if (new_value & 0x80000000)
//     {
//         new_value |= ~0xFFFFFFFF;
//     }
//     if (*prop_typed_ptr != new_value)
//     {
//         *prop_typed_ptr = new_value;
//         return true;
//     }
//     return false;
// }

// bool set_uint24_property(void *prop_ptr, uint64_t raw_value)
// {
//     auto *prop_typed_ptr = reinterpret_cast<uint32_t *>(prop_ptr);
//     uint32_t new_value = static_cast<uint32_t>(raw_value & 0xFFFFFF);
//     if (*prop_typed_ptr != new_value)
//     {
//         *prop_typed_ptr = new_value;
//         return true;
//     }
//     return false;
// }

// bool set_int48_property(void *prop_ptr, uint64_t raw_value)
// {
//     auto *prop_typed_ptr = reinterpret_cast<int64_t *>(prop_ptr);
//     int64_t new_value = static_cast<int64_t>(raw_value & 0xFFFFFFFFFFFF);
//     // Sign extension for 48-bit signed integer
//     if (new_value & 0x800000000000)
//     {
//         new_value |= ~0xFFFFFFFFFFFF;
//     }
//     if (*prop_typed_ptr != new_value)
//     {
//         *prop_typed_ptr = new_value;
//         return true;
//     }
//     return false;
// }

// {"calibration_status", {"calibration_status", PayloadType::UINT8, offsetof(MotorProperties, calibration_status), get_payload_type_size(PayloadType::UINT8), true}},
// {"current_speed", {"current_speed", PayloadType::UINT32, offsetof(MotorProperties, current_speed), get_payload_type_size(PayloadType::UINT32), false}},
// {"current_position", {"current_position", PayloadType::INT32, offsetof(MotorProperties, current_position), get_payload_type_size(PayloadType::INT32), false}},
// {"target_position", {"target_position", PayloadType::INT32, offsetof(MotorProperties, target_position), get_payload_type_size(PayloadType::INT32), false}},
// {"can_id", {"can_id", PayloadType::UINT8, offsetof(MotorProperties, can_id), get_payload_type_size(PayloadType::UINT8), false}},
// {"can_bitrate", {"can_bitrate", PayloadType::UINT8, offsetof(MotorProperties, can_bitrate), get_payload_type_size(PayloadType::UINT8), true}},
// {"group_id", {"group_id", PayloadType::UINT8, offsetof(MotorProperties, group_id), get_payload_type_size(PayloadType::UINT8), false}},
// {"working_current", {"working_current", PayloadType::UINT16, offsetof(MotorProperties, working_current), get_payload_type_size(PayloadType::UINT16), false}},
// {"holding_current", {"holding_current", PayloadType::UINT16, offsetof(MotorProperties, holding_current), get_payload_type_size(PayloadType::UINT16), false}},
// {"motor_rotation_direction", {"motor_rotation_direction", PayloadType::UINT8, offsetof(MotorProperties, motor_rotation_direction), get_payload_type_size(PayloadType::UINT8), true}},
// {"subdivisions", {"subdivisions", PayloadType::UINT8, offsetof(MotorProperties, subdivisions), get_payload_type_size(PayloadType::UINT8), false}},
// {"en_pin_config", {"en_pin_config", PayloadType::UINT8, offsetof(MotorProperties, en_pin_config), get_payload_type_size(PayloadType::UINT8), true}},
// {"key_lock_enabled", {"key_lock_enabled", PayloadType::UINT8, offsetof(MotorProperties, key_lock_enabled), get_payload_type_size(PayloadType::UINT8), true}},
// {"auto_turn_off_screen", {"auto_turn_off_screen", PayloadType::UINT8, offsetof(MotorProperties, auto_turn_off_screen), get_payload_type_size(PayloadType::UINT8), true}},
// {"locked_rotor_protection", {"locked_rotor_protection", PayloadType::UINT8, offsetof(MotorProperties, locked_rotor_protection), get_payload_type_size(PayloadType::UINT8), true}},
// {"subdivision_interpolation", {"subdivision_interpolation", PayloadType::UINT8, offsetof(MotorProperties, subdivision_interpolation), get_payload_type_size(PayloadType::UINT8), true}},
// {"home_trig", {"home_trig", PayloadType::UINT8, offsetof(MotorProperties, home_trig), get_payload_type_size(PayloadType::UINT8), true}},
// {"home_dir", {"home_dir", PayloadType::UINT8, offsetof(MotorProperties, home_dir), get_payload_type_size(PayloadType::UINT8), true}},
// {"home_speed", {"home_speed", PayloadType::UINT16, offsetof(MotorProperties, home_speed), get_payload_type_size(PayloadType::UINT16), false}},
// {"end_limit", {"end_limit", PayloadType::UINT8, offsetof(MotorProperties, end_limit), get_payload_type_size(PayloadType::UINT8), true}},
// {"emergency_stop_triggered", {"emergency_stop_triggered", PayloadType::UINT8, offsetof(MotorProperties, emergency_stop_triggered), get_payload_type_size(PayloadType::UINT8), true}},
// {"is_enabled", {"is_enabled", PayloadType::UINT8, offsetof(MotorProperties, is_enabled), get_payload_type_size(PayloadType::UINT8), true}},
// {"mode0_status", {"mode0_status", PayloadType::UINT8, offsetof(MotorProperties, mode0_status), get_payload_type_size(PayloadType::UINT8), true}},
// {"motor_shaft_protection_status", {"motor_shaft_protection_status", PayloadType::UINT8, offsetof(MotorProperties, motor_shaft_protection_status), get_payload_type_size(PayloadType::UINT8), true}},
// {"save_clean_state", {"save_clean_state", PayloadType::UINT8, offsetof(MotorProperties, save_clean_state), get_payload_type_size(PayloadType::UINT8), true}},
// {"last_seen", {"last_seen", PayloadType::VOID, offsetof(MotorProperties, last_seen), sizeof(std::chrono::system_clock::time_point), false}},
// {"dummy", {"dummy", PayloadType::VOID, offsetof(MotorProperties, dummy), sizeof(std::any), false}}
// // Add entries for all properties