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

// Define the event base for motor controller events
ESP_EVENT_DEFINE_BASE(MOTOR_CONTROLLER_EVENT);

class TWAICommandFactory;
class CommandMapper;
class TWAIController;
class CommandLifecycleRegistry;
class MotorContext;
class MotorResponseHandler;

struct TWAICommandFactorySettings
{
    uint32_t id;
    QueueHandle_t outQ;
    QueueHandle_t inQ;
    std::shared_ptr<CommandLifecycleRegistry> command_lifecycle_registry;

    // Constructor
    TWAICommandFactorySettings(uint32_t id, QueueHandle_t outQ, QueueHandle_t inQ, std::shared_ptr<CommandLifecycleRegistry> command_lifecycle_registry)
        : id(id), outQ(outQ), inQ(inQ), command_lifecycle_registry(command_lifecycle_registry)
    {
        ESP_LOGI("TWAICommandFactorySettings", "Constructor called");
    }

    // Destructor
    ~TWAICommandFactorySettings()
    {
        ESP_LOGI("TWAICommandFactorySettings", "Destructor called");
    }
};

enum MotorEvent
{
    MOTOR_EVENT_INIT,
    MOTOR_EVENT_READY,
    MOTOR_EVENT_RUNNING,
    MOTOR_EVENT_ERROR,
    MOTOR_EVENT_RECOVERING
};

struct MotorControllerDependencies
{
    uint32_t id;
    QueueHandle_t inQ;
    QueueHandle_t outQ;
    EventGroupHandle_t system_event_group;
    std::shared_ptr<CommandMapper> command_mapper;
    std::shared_ptr<CommandLifecycleRegistry> command_lifecycle_registry;
    std::shared_ptr<TWAICommandFactory> command_factory;
    std::shared_ptr<MotorContext> motor_context;
    std::shared_ptr<MotorResponseHandler> motor_response_handler;

    // Constructor
    MotorControllerDependencies(
        uint32_t id,
        QueueHandle_t inQ,
        QueueHandle_t outQ,
        EventGroupHandle_t system_event_group,
        std::shared_ptr<CommandMapper> command_mapper,
        std::shared_ptr<CommandLifecycleRegistry> command_lifecycle_registry,
        std::shared_ptr<TWAICommandFactory> command_factory,
        std::shared_ptr<MotorContext> motor_context,
        std::shared_ptr<MotorResponseHandler> motor_response_handler)
        : id(id), inQ(inQ), outQ(outQ), system_event_group(system_event_group), command_mapper(command_mapper), command_lifecycle_registry(command_lifecycle_registry), command_factory(command_factory), motor_context(motor_context), motor_response_handler(motor_response_handler)
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