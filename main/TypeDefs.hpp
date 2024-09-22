#ifndef TYPEDEFS_HPP
#define TYPEDEFS_HPP

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <memory>

class TWAICommandFactory;
class CommandMapper;
class TWAIController;
class CommandLifecycleRegistry;

typedef struct
{
    uint32_t id;
    QueueHandle_t outQ;
    QueueHandle_t inQ;
    std::shared_ptr<CommandLifecycleRegistry> command_lifecycle_registry;
} TWAICommandFactorySettings;

typedef struct
{
    uint32_t id;
    QueueHandle_t inQ;
    QueueHandle_t outQ;
    std::shared_ptr<CommandMapper> command_mapper;
    std::shared_ptr<CommandLifecycleRegistry> command_lifecycle_registry;
    std::shared_ptr<TWAICommandFactory> command_factory;
} MotorControllerDependencies;

#endif // TYPEDEFS_HPP