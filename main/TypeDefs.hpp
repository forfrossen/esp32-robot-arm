#ifndef TYPEDEFS_HPP
#define TYPEDEFS_HPP

#include <esp_event.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <memory>

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
class MotorResponseHandler;

struct EventLoops
{
    esp_event_loop_handle_t system_event_loop;
    esp_event_loop_handle_t motor_event_loop;
};

struct EventGroups
{
    EventGroupHandle_t system_event_group;
    EventGroupHandle_t motor_event_group;
};

struct TWAIQueues
{
    QueueHandle_t inQ;
    QueueHandle_t outQ;
};

struct TWAICommandFactorySettings
{
    uint32_t id;
    std::shared_ptr<TWAIQueues> twai_queues;
    std::shared_ptr<CommandLifecycleRegistry> command_lifecycle_registry;

    // Constructor
    TWAICommandFactorySettings(uint32_t id, std::shared_ptr<TWAIQueues> twai_queues, std::shared_ptr<CommandLifecycleRegistry> command_lifecycle_registry)
        : id(id), twai_queues(twai_queues), command_lifecycle_registry(command_lifecycle_registry)
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
    RECEIVED,
    PROCESSED,
    ERROR,
    TIMEOUT,
    UNKNOWN
};

typedef enum
{
    MOTOR_EVENT_INIT,
    MOTOR_EVENT_READY,
    MOTOR_EVENT_RUNNING,
    MOTOR_EVENT_ERROR,
    MOTOR_EVENT_RECOVERING
} motor_event_id_t;

typedef enum
{
    TWAI_READY_EVENT,
    WIFI_READY_EVENT,
    WEBSOCKET_READY_EVENT,
    TWAI_ERROR_EVENT,
    WIFI_ERROR_EVENT,
    WEBSOCKET_ERROR_EVENT
} system_event_id_t;

struct MotorControllerDependencies
{
    uint32_t id;
    SemaphoreHandle_t motor_mutex;
    std::shared_ptr<TWAIQueues> twai_queues;
    std::shared_ptr<EventGroups> event_groups;
    std::shared_ptr<EventLoops> event_loops;
    std::shared_ptr<CommandLifecycleRegistry> command_lifecycle_registry;
    std::shared_ptr<TWAICommandFactory> command_factory;
    std::shared_ptr<MotorContext> motor_context;
    std::shared_ptr<MotorResponseHandler> motor_response_handler;

    // Constructor
    MotorControllerDependencies(
        uint32_t id,
        SemaphoreHandle_t motor_mutex,
        std::shared_ptr<TWAIQueues> twai_queues,
        std::shared_ptr<EventGroups> event_groups,
        std::shared_ptr<EventLoops> event_loops,
        std::shared_ptr<CommandLifecycleRegistry> command_lifecycle_registry,
        std::shared_ptr<TWAICommandFactory> command_factory,
        std::shared_ptr<MotorContext> motor_context,
        std::shared_ptr<MotorResponseHandler> motor_response_handler)
        : id(id),
          motor_mutex(motor_mutex),
          twai_queues(twai_queues),
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

#endif // TYPEDEFS_HPP