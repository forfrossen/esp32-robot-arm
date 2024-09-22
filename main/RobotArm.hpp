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
#include <memory>

class RobotArm
{
private:
    std::map<uint8_t, std::shared_ptr<MotorController>> servos;
    const std::shared_ptr<TWAIController> twai_controller = std::make_shared<TWAIController>();
    const std::shared_ptr<CommandMapper> command_mapper = std::make_shared<CommandMapper>();
    const std::shared_ptr<CommandLifecycleRegistry> command_lifecyle_registry = std::make_shared<CommandLifecycleRegistry>();

public:
    RobotArm()
    {
        ESP_LOGI(FUNCTION_NAME, "RobotArm constructor called");

        // twai_controller = std::make_shared<TWAIController>();
        // command_mapper = std::make_shared<CommandMapper>();
        // command_lifecyle_registry = std::make_shared<CommandLifecycleRegistry>();

        for (int i = 1; i < 4; i++)
        {
            if (i == 2)
            {
                const std::shared_ptr<MotorControllerDependencies> motor_controller_dependencies = motor_controller_dependencies_init(i);
                servos[i] = std::make_shared<MotorController>(MotorController(motor_controller_dependencies));
            }
        }
        // Servos[0x01] = new MotorController(0x01, twai_controller, command_mapper);
        // servos[2] = new MotorController(0x02, twai_controller, command_mapper);
        // Servos[0x03] = new MotorController(0x03, twai_controller, command_mapper);
    }

    const std::shared_ptr<MotorControllerDependencies> motor_controller_dependencies_init(uint32_t id)
    {
        QueueHandle_t inQ = xQueueCreate(10, sizeof(twai_message_t));

        auto settings = std::make_shared<TWAICommandFactorySettings>();
        settings->id = id;
        settings->outQ = twai_controller->outQ;
        settings->inQ = inQ;
        settings->command_lifecycle_registry = command_lifecyle_registry;

        const std::shared_ptr<MotorControllerDependencies>
            container = std::make_shared<MotorControllerDependencies>(MotorControllerDependencies{
                .id = id,
                .inQ = inQ,
                .outQ = twai_controller->outQ,
                .command_mapper = command_mapper,
                .command_lifecycle_registry = command_lifecyle_registry,
                .command_factory = std::make_shared<TWAICommandFactory>(settings),
            });
        return container;
    }
};
#endif // ROBOTARM_HPP