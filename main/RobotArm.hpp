#ifndef ROBOTARM_HPP
#define ROBOTARM_HPP

#include "CommandMapper.hpp"
#include "Commands/CommandLifecycleRegistry.hpp"
#include "Commands/TWAICommandFactory.hpp"
#include "MotorController.hpp"
#include "TWAIController.hpp"
#include "TypeDefs.hpp"
#include "freeRTOS/queue.h"
#include "freertos/FreeRTOS.h"

struct ServoRegistryEntry
{
    const MotorController *motorController;
    const SpecificServices *specificServices;
};

class RobotArm
{
private:
    std::map<uint8_t, ServoRegistryEntry> servos;
    TWAIController *twai_controller = new TWAIController();
    CommandMapper *command_mapper = new CommandMapper();
    CommandLifecycleRegistry *command_lifecyle_registry = new CommandLifecycleRegistry();

public:
    RobotArm()
    {
        ESP_LOGI(FUNCTION_NAME, "RobotArm constructor called");

        for (int i = 1; i < 4; i++)
        {
            if (i == 2)
            {

                const SpecificServices *specific_services = specific_services_init(i);
                servos[i] = {.motorController = new MotorController(i, specific_services),
                             .specificServices = specific_services};
            }
        }
        // Servos[0x01] = new MotorController(0x01, twai_controller, command_mapper);
        // servos[2] = new MotorController(0x02, twai_controller, command_mapper);
        // Servos[0x03] = new MotorController(0x03, twai_controller, command_mapper);
    }

    ~RobotArm()
    {
        // Speicherfreigabe für die Servo-Objekte
        for (auto &servo : servos)
        {
            delete servo.second.motorController;
            delete servo.second.specificServices;
        }

        // Speicherfreigabe für die spezifischen Dienste
        delete twai_controller;
        delete command_mapper;
        delete command_lifecyle_registry;
    }

    SpecificServices *specific_services_init(uint32_t id)
    {
        // SpecificServices *container = static_cast<SpecificServices *>(malloc(sizeof(SpecificServices)));
        QueueHandle_t inQ = xQueueCreate(10, sizeof(twai_message_t));
        TWAICommandFactorySettings settings = {.id = id, .outQ = twai_controller->outQ, .inQ = inQ, .command_lifecycle_registry = command_lifecyle_registry};
        TWAICommandFactory *command_factory = new TWAICommandFactory(settings);
        SpecificServices *container = new SpecificServices({
            .inQ = inQ,
            .command_factory = command_factory,
            .command_mapper = command_mapper,
            .command_lifecycle_registry = command_lifecyle_registry,
        });
        return container;
    }
};
#endif // ROBOTARM_HPP