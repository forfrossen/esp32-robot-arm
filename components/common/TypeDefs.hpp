#ifndef TYPEDEFS_HPP
#define TYPEDEFS_HPP

#include <array>
#include <esp_event.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <map>
#include <memory>
#include <optional>
#include <type_traits>
#include <variant>

#include "MksEnums.hpp"

#define MAX_PROCESSED_MESSAGES 10

// Motor constants
#define STEPS_PER_REVOLUTION 16384
#define ACTUATOR_GEAR_RATIO 38.4
// #define STEPS_PER_REVOLUTION 256

// Event group bits
// SYSTEM BITS
#define TWAI_READY BIT0
#define SYSTEM_ERROR BIT1

// MOTOR BITS
#define MOTOR_READY_BIT BIT0
#define MOTOR_ERROR_BIT BIT1
#define MOTOR_RECOVERING_BIT BIT2
#define MOTOR_INIT_BIT BIT3

class CommandFactory;
class TWAIController;
class CommandLifecycleRegistry;
class MotorContext;
class ResponseHandler;

typedef struct EventLoops
{
    esp_event_loop_handle_t system_event_loop;
    esp_event_loop_handle_t motor_event_loop;
    EventLoops(esp_event_loop_handle_t system_event_loop, esp_event_loop_handle_t motor_event_loop)
        : system_event_loop(system_event_loop), motor_event_loop(motor_event_loop)
    {
        ESP_LOGI("EventLoops", "Constructor called");
    }
} event_loops_t;

typedef struct EventGroups
{
    EventGroupHandle_t system_event_group;
    EventGroupHandle_t motor_event_group;
    EventGroups(EventGroupHandle_t system_event_group, EventGroupHandle_t motor_event_group)
        : system_event_group(system_event_group), motor_event_group(motor_event_group)
    {
        ESP_LOGI("EventGroups", "Constructor called");
    }
} event_groups_t;

struct CommandFactorySettings
{
    uint32_t id;
    esp_event_loop_handle_t system_event_loop;
    std::shared_ptr<CommandLifecycleRegistry> command_lifecycle_registry;

    // Constructor
    CommandFactorySettings(uint32_t id, esp_event_loop_handle_t system_event_loop, std::shared_ptr<CommandLifecycleRegistry> command_lifecycle_registry)
        : id(id), system_event_loop(system_event_loop), command_lifecycle_registry(command_lifecycle_registry)
    {
        ESP_LOGI("CommandFactorySettings", "Constructor called");
    }

    // Destructor
    ~CommandFactorySettings()
    {
        ESP_LOGI("CommandFactorySettings", "Destructor called");
    }
};

enum class CommandLifecycleState
{
    CREATED,
    SENT,
    EXECUTING,
    PROCESSED,
    ERROR,
    TIMEOUT,
    UNKNOWN
};

typedef enum
{
    STATE_TRANSITION_EVENT,
    INCOMING_MESSAGE_EVENT,
    OUTGOING_MESSAGE_EVENT,
} motor_event_id_t;

typedef enum
{
    ARM_INITIALIZING,
    TWAI_READY_EVENT,
    WIFI_READY_EVENT,
    WEBSOCKET_READY_EVENT,
    TWAI_ERROR_EVENT,
    WIFI_ERROR_EVENT,
    WEBSOCKET_ERROR_EVENT,
    PROPERTY_CHANGE_EVENT,
} system_event_id_t;

struct MotorControllerDependencies
{
    uint32_t id;
    SemaphoreHandle_t motor_mutex;

    std::shared_ptr<EventGroups> event_groups;
    std::shared_ptr<EventLoops> event_loops;
    std::shared_ptr<CommandLifecycleRegistry> command_lifecycle_registry;
    std::shared_ptr<CommandFactory> command_factory;
    std::shared_ptr<MotorContext> motor_context;
    std::shared_ptr<ResponseHandler> motor_response_handler;

    // Constructor
    MotorControllerDependencies(
        uint32_t id,
        SemaphoreHandle_t motor_mutex,
        std::shared_ptr<EventGroups> event_groups,
        std::shared_ptr<EventLoops> event_loops,
        std::shared_ptr<CommandLifecycleRegistry> command_lifecycle_registry,
        std::shared_ptr<CommandFactory> command_factory,
        std::shared_ptr<MotorContext> motor_context,
        std::shared_ptr<ResponseHandler> motor_response_handler)
        : id(id),
          motor_mutex(motor_mutex),
          event_groups(event_groups),
          event_loops(event_loops),
          command_lifecycle_registry(command_lifecycle_registry),
          command_factory(command_factory),
          motor_context(motor_context),
          motor_response_handler(motor_response_handler)
    {
        ESP_LOGI("MotorControllerDependencies", "Constructor called");
    }

    ~MotorControllerDependencies()
    {
        ESP_LOGI("MotorControllerDependencies", "Destructor called");
    }
};

using uint4_t = uint8_t;
using uint12_t = uint16_t;
using uint24_t = uint32_t;
using int24_t = int32_t;

enum class PayloadType
{
    VOID,
    UINT4,
    UINT8,
    UINT16,
    UINT24,
    UINT32,
    UINT48,
    INT16,
    INT24,
    INT32,
    INT48,
};

// using CommandPayloadTypes = std::variant<uint8_t, uint16_t, uint32_t, uint64_t>; // Defined variant for all possible types
// using PayloadInfoMap = std::map<CommandIds, std::tuple<CommandPayloadTypes>>;    // Payloads with relevant data types

// // using PayloadInfoMap = std::map<CommandIds, std::make_tuple()>;
// const PayloadInfoMap command_payload_map = {
//     {MOTOR_CALIBRATION, std::make_tuple()},
//     {READ_MOTOR_SPEED, std::make_tuple()},
//     {EMERGENCY_STOP, std::make_tuple()},
//     {READ_ENCODER_VALUE_CARRY, std::make_tuple()},
//     {READ_ENCODED_VALUE_ADDITION, std::make_tuple()},
//     {READ_NUM_PULSES_RECEIVED, std::make_tuple()},
//     {READ_IO_PORT_STATUS, std::make_tuple()},
//     {READ_MOTOR_SHAFT_ANGLE_ERROR, std::make_tuple()},
//     {READ_EN_PINS_STATUS, std::make_tuple()},
//     {READ_GO_BACK_TO_ZERO_STATUS_WHEN_POWER_ON, std::make_tuple()},
//     {RELEASE_MOTOR_SHAFT_LOCKED_PROTECTION_STATE, std::make_tuple()},
//     {READ_MOTOR_SHAFT_PROTECTION_STATE, std::make_tuple()},
//     {RESTORE_DEFAULT_PARAMETERS, std::make_tuple()},
//     {RESTART, std::make_tuple()},
//     {GO_HOME, std::make_tuple()},
//     {SET_CURRENT_AXIS_TO_ZERO, std::make_tuple()},
//     {QUERY_MOTOR_STATUS, std::make_tuple()},
//     {ENABLE_MOTOR, std::make_tuple()},

//     // Commands with a single parameter as payload
//     {SET_WORKING_CURRENT, std::make_tuple(uint8_t{}, uint16_t{})},
//     {SET_SUBDIVISIONS, std::make_tuple(uint8_t{}, uint8_t{})},
//     {SET_EN_PIN_CONFIG, std::make_tuple(uint8_t{}, uint8_t{})},
//     {SET_MOTOR_ROTATION_DIRECTION, std::make_tuple(uint8_t{}, uint8_t{})},
//     {SET_AUTO_TURN_OFF_SCREEN, std::make_tuple(uint8_t{}, uint8_t{})},
//     {SET_MOTOR_SHAFT_LOCKED_ROTOR_PROTECTION, std::make_tuple(uint8_t{}, uint8_t{})},
//     {SET_SUBDIVISION_INTERPOLATION, std::make_tuple(uint8_t{}, uint8_t{})},
//     {SET_CAN_BITRATE, std::make_tuple(uint8_t{}, uint8_t{})},
//     {SET_CAN_ID, std::make_tuple(uint8_t{}, uint16_t{})},
//     {SET_KEY_LOCK_ENABLE, std::make_tuple(uint8_t{}, uint8_t{})},

//     {SET_HOME, std::make_tuple(uint8_t{}, uint8_t{}, uint8_t{}, uint16_t{}, uint8_t{})},

//     //                                              code    speed     acc      pos
//     {RUN_MOTOR_ABSOLUTE_MOTION_BY_AXIS, std::make_tuple(uint8_t{}, uint16_t{}, uint8_t{}, uint24_t{})},
//     {RUN_MOTOR_RELATIVE_MOTION_BY_AXIS, std::make_tuple(uint8_t{}, uint16_t{}, uint8_t{}, uint24_t{})}};

inline size_t get_payload_type_size(PayloadType type)
{
    switch (type)
    {
    case PayloadType::VOID:
        return 0;
    case PayloadType::UINT4:
        return 1; // Handle packing of half-byte values manually.
    case PayloadType::UINT8:
        return 1;
    case PayloadType::UINT16:
        return 2;
    case PayloadType::UINT24:
        return 3;
    case PayloadType::UINT32:
        return 4;
    case PayloadType::UINT48:
        return 6;
    case PayloadType::INT16:
        return 2;
    case PayloadType::INT24:
        return 3;
    case PayloadType::INT32:
        return 4;
    case PayloadType::INT48:
        return 6;
    default:
        return 0;
    }
}

struct CommandPayloadInfo
{
    std::array<PayloadType, 7> payload_types;

    template <typename... Types>
    CommandPayloadInfo(Types... types)
    {
        static_assert(sizeof...(types) <= 7, "Too many payload types provided, max is 7");

        static_assert((... && std::is_same_v<Types, PayloadType>),
                      "All arguments must be of type PayloadType");
        std::size_t index = 0;

        ((payload_types[index++] = types), ...);

        while (index < payload_types.size())
        {
            payload_types[index++] = PayloadType::VOID;
        }
    }

    PayloadType getType(std::size_t index) const
    {
        if (index < payload_types.size())
        {
            return payload_types[index];
        }
        else
        {
            return PayloadType::VOID;
        }
    }
};

struct CommonPayload
{
    uint8_t command_id;
};

struct CommonReturnPayload
{
    uint8_t command_id;
    uint8_t status;
};

const std::map<CommandIds, CommandPayloadInfo> g_command_payload_map = {
    {MOTOR_CALIBRATION, CommandPayloadInfo()},
    {READ_MOTOR_SPEED, CommandPayloadInfo()},
    {EMERGENCY_STOP, CommandPayloadInfo()},
    {READ_ENCODER_VALUE_CARRY, CommandPayloadInfo()},
    {READ_ENCODED_VALUE_ADDITION, CommandPayloadInfo()},
    {READ_MOTOR_SPEED, CommandPayloadInfo()},
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

const std::map<CommandIds, std::optional<CommandPayloadInfo>> ResponsePayloadMap = {
    // Commands with standard 2-byte response (CommandId and Status)
    {MOTOR_CALIBRATION, CommandPayloadInfo(PayloadType::UINT8)},                           // Status only
    {EMERGENCY_STOP, CommandPayloadInfo(PayloadType::UINT8)},                              // Status only
    {QUERY_MOTOR_STATUS, CommandPayloadInfo(PayloadType::UINT8)},                          // Status only
    {ENABLE_MOTOR, CommandPayloadInfo(PayloadType::UINT8)},                                // Status only
    {GO_HOME, CommandPayloadInfo(PayloadType::UINT8)},                                     // Status only
    {SET_CURRENT_AXIS_TO_ZERO, CommandPayloadInfo(PayloadType::UINT8)},                    // Status only
    {RESTORE_DEFAULT_PARAMETERS, CommandPayloadInfo(PayloadType::UINT8)},                  // Status only
    {RELEASE_MOTOR_SHAFT_LOCKED_PROTECTION_STATE, CommandPayloadInfo(PayloadType::UINT8)}, // Status only
    {READ_MOTOR_SHAFT_PROTECTION_STATE, CommandPayloadInfo(PayloadType::UINT8)},           // Status only
    {READ_EN_PINS_STATUS, CommandPayloadInfo(PayloadType::UINT8)},                         // Status only
    {READ_IO_PORT_STATUS, CommandPayloadInfo(PayloadType::UINT8)},                         // Status only
    {READ_GO_BACK_TO_ZERO_STATUS_WHEN_POWER_ON, CommandPayloadInfo(PayloadType::UINT8)},   // Status only
    {SET_WORKING_CURRENT, CommandPayloadInfo(PayloadType::UINT8)},                         // Status only
    {SET_SUBDIVISIONS, CommandPayloadInfo(PayloadType::UINT8)},                            // Status only
    {SET_EN_PIN_CONFIG, CommandPayloadInfo(PayloadType::UINT8)},                           // Status only
    {SET_MOTOR_ROTATION_DIRECTION, CommandPayloadInfo(PayloadType::UINT8)},                // Status only
    {SET_AUTO_TURN_OFF_SCREEN, CommandPayloadInfo(PayloadType::UINT8)},                    // Status only
    {SET_MOTOR_SHAFT_LOCKED_ROTOR_PROTECTION, CommandPayloadInfo(PayloadType::UINT8)},     // Status only
    {SET_SUBDIVISION_INTERPOLATION, CommandPayloadInfo(PayloadType::UINT8)},               // Status only
    {SET_CAN_BITRATE, CommandPayloadInfo(PayloadType::UINT8)},                             // Status only
    {SET_CAN_ID, CommandPayloadInfo(PayloadType::UINT8)},                                  // Status only
    {SET_KEY_LOCK_ENABLE, CommandPayloadInfo(PayloadType::UINT8)},                         // Status only
    {SET_HOME, CommandPayloadInfo(PayloadType::UINT8)},                                    // Status only
    {RUN_MOTOR_SPEED_MODE, CommandPayloadInfo(PayloadType::UINT8)},                        // Status only
    {RUN_MOTOR_ABSOLUTE_MOTION_BY_AXIS, CommandPayloadInfo(PayloadType::UINT8)},           // Speed, acceleration, and absolute axis (uint24)
    {RUN_MOTOR_RELATIVE_MOTION_BY_AXIS, CommandPayloadInfo(PayloadType::UINT8)},           // Speed, acceleration, and relative axis (uint24)
    {RUN_MOTOR_RELATIVE_MOTION_BY_PULSES, CommandPayloadInfo(PayloadType::UINT8)},         // Speed, acceleration, relative pulses (uint24)
    {RUN_MOTOR_ABSOLUTE_MOTION_BY_PULSES, CommandPayloadInfo(PayloadType::UINT8)},         // Speed, acceleration, absolute pulses (uint24)

    // Commands with more complex responses
    {READ_ENCODER_VALUE_CARRY, CommandPayloadInfo(PayloadType::INT32, PayloadType::UINT16)}, // Carry (int32) and encoder value (uint16)
    {READ_ENCODED_VALUE_ADDITION, CommandPayloadInfo(PayloadType::INT48)},                   // Encoder value (int48)
    {READ_MOTOR_SPEED, CommandPayloadInfo(PayloadType::INT16)},                              // Speed (int16)
    {READ_NUM_PULSES_RECEIVED, CommandPayloadInfo(PayloadType::INT32)},                      // Number of pulses (int32)
    {READ_MOTOR_SHAFT_ANGLE_ERROR, CommandPayloadInfo(PayloadType::INT32)},                  // Shaft angle error (int32)
};

#endif // TYPEDEFS_HPP