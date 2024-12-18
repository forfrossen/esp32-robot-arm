#ifndef TYPEDEFS_HPP
#define TYPEDEFS_HPP

#include "MksEnums.hpp"

#include <any>
#include <array>
#include <esp_event.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <map>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

#define MAX_PROCESSED_MESSAGES 10

// Motor constants
#define STEPS_PER_REVOLUTION 16384
#define ACTUATOR_GEAR_RATIO 38.4
// #define STEPS_PER_REVOLUTION 256

// Event group bits
// SYSTEM ERROR BITS
#define SYSTEM_ERROR BIT0
#define TWAI_ERROR BIT1
#define WIFI_ERROR BIT2
#define WEBSOCKET_ERROR BIT3
// SYSTTEM READY BITS
#define TWAI_READY BIT4
#define WIFI_READY BIT5
#define WEBSOCKET_READY BIT6
// RUNLEVEL BITS
#define RUNLEVEL_0 BIT7  // System initialization
#define RUNLEVEL_1 BIT8  // Query status
#define RUNLEVEL_2 BIT9  // Query position
#define RUNLEVEL_3 BIT10 // Move Motors

// MOTOR BITS
#define MOTOR_READY_BIT BIT0
#define MOTOR_ERROR_BIT BIT1
#define MOTOR_RECOVERING_BIT BIT2
#define MOTOR_INIT_BIT BIT3
#define MOTOR_IDLE_BIT BIT4

class CommandFactory;
class TWAIController;
class CommandLifecycleRegistry;
class MotorContext;
class ResponseHandler;

typedef struct EventLoops
{
    esp_event_loop_handle_t system_event_loop;
    esp_event_loop_handle_t motor_event_loop;
    EventLoops(
        esp_event_loop_handle_t system_event_loop,
        esp_event_loop_handle_t motor_event_loop)
        : system_event_loop(system_event_loop), motor_event_loop(motor_event_loop)
    {
        ESP_LOGD("EventLoops", "Constructor called");
    }
} event_loops_t;

typedef struct EventGroups
{
    EventGroupHandle_t system_event_group;
    EventGroupHandle_t motor_event_group;
    EventGroups(
        EventGroupHandle_t &system_event_group,
        EventGroupHandle_t &motor_event_group)
        : system_event_group(system_event_group),
          motor_event_group(motor_event_group)
    {
        ESP_LOGD("EventGroups", "Constructor called");
    }
} event_groups_t;

struct CommandFactorySettings
{
    uint32_t id;
    esp_event_loop_handle_t system_event_loop;
    std::shared_ptr<CommandLifecycleRegistry> command_lifecycle_registry;

    // Constructor
    CommandFactorySettings(
        uint32_t id,
        esp_event_loop_handle_t system_event_loop,
        std::shared_ptr<CommandLifecycleRegistry> command_lifecycle_registry)
        : id(id),
          system_event_loop(system_event_loop),
          command_lifecycle_registry(command_lifecycle_registry)
    {
        ESP_LOGD("CommandFactorySettings", "Constructor called");
    }

    // Destructor
    ~CommandFactorySettings()
    {
        ESP_LOGD("CommandFactorySettings", "Destructor called");
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

using NextStateTransitions = std::variant<CommandLifecycleState>;

typedef enum
{
    STATE_TRANSITION_EVENT,
    INCOMING_MESSAGE_EVENT,
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
    OUTGOING_MESSAGE_EVENT,
    REMOTE_CONTROL_EVENT,
    SET_RUN_MODE_EVENT,
    MOTOR_CONTROL_COMMAND,
} system_event_id_t;

typedef enum
{
    SEND_RESPONSE,
} rpc_event_id_t;

typedef enum
{
    START_MOTORS,
    STOP_MOTORS,
} remote_control_event_t;

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
        ESP_LOGD("MotorControllerDependencies", "Constructor called");
    }

    ~MotorControllerDependencies()
    {
        ESP_LOGD("MotorControllerDependencies", "Destructor called");
    }
};

enum class RunLevel
{
    RUNLEVEL0 = 0,
    RUNLEVEL1 = 1,
    RUNLEVEL2 = 2,
    RUNLEVEL3 = 3,
    UNKNOWN = 99
};

#endif // TYPEDEFS_HPP