#ifndef TYPEDEFS_HPP
#define TYPEDEFS_HPP

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

class TWAICommandFactory;
class CommandMapper;
class TWAIController;
class CommandLifecycleRegistry;

typedef struct
{
    uint32_t id;
    QueueHandle_t outQ;
    QueueHandle_t inQ;
    CommandLifecycleRegistry *command_lifecycle_registry;
} TWAICommandFactorySettings;

typedef struct
{
    QueueHandle_t inQ;
    QueueHandle_t outQ;
    TWAICommandFactory *command_factory;
    CommandMapper *command_mapper;
    CommandLifecycleRegistry *command_lifecycle_registry;
} SpecificServices;

#endif // TYPEDEFS_HPP