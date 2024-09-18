#ifndef TYPEDEFS_HPP
#define TYPEDEFS_HPP

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

class TWAICommandFactory;
class CommandMapper;
class TWAIController;

typedef struct
{
    uint32_t id;
    QueueHandle_t outQ;
    QueueHandle_t inQ;
} TWAICommandFactorySettings;

typedef struct
{
    TWAIController *twai_controller;
    CommandMapper *command_mapper;
} SharedServices;

typedef struct
{
    QueueHandle_t inQ;
    TWAICommandFactory *command_factory;
} SpecificServices;

#endif // TYPEDEFS_HPP