#ifndef ROBOTARM_HPP
#define ROBOTARM_HPP

#include "CommandMapper.hpp"
#include "Commands/TWAICommandFactory.hpp"
#include "MotorController.hpp"
#include "TWAIController.hpp"
#include "TypeDefs.hpp"
#include "freeRTOS/queue.h"
#include "freertos/FreeRTOS.h"
class RobotArm
{
private:
    std::map<uint8_t, MotorController *> servos;
    SharedServices *shared_services;

public:
    RobotArm()
    {
        ESP_LOGI(FUNCTION_NAME, "RobotArm constructor called");

        shared_services = shared_services_init();

        for (int i = 1; i < 4; i++)
        {
            if (i == 2)
            {

                SpecificServices *specific_services = specific_services_init(i, shared_services->twai_controller->outQ);
                servos[i] = new MotorController(i, shared_services, specific_services);
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
            delete servo.second;
        }
    }

    SharedServices *shared_services_init()
    {
        // SharedServices *container = static_cast<SharedServices *>(malloc(sizeof(SharedServices)));
        SharedServices *container = new SharedServices();
        container->twai_controller = new TWAIController();
        container->command_mapper = new CommandMapper();
        return container;
    }

    SpecificServices *specific_services_init(uint32_t id, QueueHandle_t outQ)
    {
        // SpecificServices *container = static_cast<SpecificServices *>(malloc(sizeof(SpecificServices)));
        SpecificServices *container = new SpecificServices();
        container->inQ = xQueueCreate(10, sizeof(twai_message_t));
        TWAICommandFactorySettings settings{id, container->inQ, outQ};
        container->command_factory = new TWAICommandFactory(settings);
        return container;
    }
};
#endif // ROBOTARM_HPP