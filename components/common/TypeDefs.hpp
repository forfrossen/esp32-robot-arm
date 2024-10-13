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

#include "MksEnums.hpp"

#define MAX_PROCESSED_MESSAGES 10

// Motor constants
#define STEPS_PER_REVOLUTION 16384

// Event group bits
// SYSTEM BITS
#define TWAI_READY BIT0
#define SYSTEM_ERROR BIT1

// MOTOR BITS
#define MOTOR_READY_BIT BIT0
#define MOTOR_ERROR_BIT BIT1
#define MOTOR_RECOVERING_BIT BIT2
#define MOTOR_INIT_BIT BIT3

class TWAICommandFactory;
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

struct TWAICommandFactorySettings
{
    uint32_t id;
    esp_event_loop_handle_t system_event_loop;
    std::shared_ptr<CommandLifecycleRegistry> command_lifecycle_registry;

    // Constructor
    TWAICommandFactorySettings(uint32_t id, std::shared_ptr<CommandLifecycleRegistry> command_lifecycle_registry)
        : id(id), command_lifecycle_registry(command_lifecycle_registry)
    {
        ESP_LOGI("TWAICommandFactorySettings", "Constructor called");
    }

    // Destructor
    ~TWAICommandFactorySettings()
    {
        ESP_LOGI("TWAICommandFactorySettings", "Destructor called");
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
    std::shared_ptr<TWAICommandFactory> command_factory;
    std::shared_ptr<MotorContext> motor_context;
    std::shared_ptr<ResponseHandler> motor_response_handler;

    // Constructor
    MotorControllerDependencies(
        uint32_t id,
        SemaphoreHandle_t motor_mutex,
        std::shared_ptr<EventGroups> event_groups,
        std::shared_ptr<EventLoops> event_loops,
        std::shared_ptr<CommandLifecycleRegistry> command_lifecycle_registry,
        std::shared_ptr<TWAICommandFactory> command_factory,
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

    // Destructor
    ~MotorControllerDependencies()
    {
        ESP_LOGI("MotorControllerDependencies", "Destructor called");
    }
};

enum class PayloadType
{
    VOID,
    UINT8,
    UINT16,
    UINT32,
    // Add more types as needed
};

struct CommandPayloadInfo
{
    std::array<PayloadType, 7> payload_types;

    // Variadic template constructor
    template <typename... Types>
    CommandPayloadInfo(Types... types)
    {
        static_assert(sizeof...(types) <= 7, "Too many payload types provided, max is 7");
        std::size_t index = 0;
        // Initialize with provided types
        ((payload_types[index++] = types), ...);
        // Fill remaining slots with VOID
        while (index < payload_types.size())
        {
            payload_types[index++] = PayloadType::VOID;
        }
    }

    // Helper function to get the payload type at an index
    PayloadType getType(std::size_t index) const
    {
        if (index < payload_types.size())
        {
            return payload_types[index];
        }
        else
        {
            return PayloadType::VOID; // Return VOID if out of bounds
        }
    }
};

typedef std::map<CommandIds, std::optional<CommandPayloadInfo>> CommandPayloadMap;

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

    {SET_HOME, CommandPayloadInfo(PayloadType::UINT8, PayloadType::UINT8, PayloadType::UINT16, PayloadType::UINT8)}};
#endif // TYPEDEFS_HPP