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
    EventGroupHandle_t system_event_group;
    std::map<uint8_t, MotorController *> servos;
    const std::shared_ptr<TWAIController> twai_controller = std::make_shared<TWAIController>();
    const std::shared_ptr<CommandMapper> command_mapper = std::make_shared<CommandMapper>();
    const std::shared_ptr<CommandLifecycleRegistry> command_lifecyle_registry = std::make_shared<CommandLifecycleRegistry>();

public:
    RobotArm()
    {
        ESP_LOGI(FUNCTION_NAME, "RobotArm constructor called");
        system_event_group = xEventGroupCreate();
        if (system_event_group == NULL)
        {
            ESP_LOGE(FUNCTION_NAME, "Error creating event group");
        }

        for (int i = 1; i < 4; i++)
        {
            if (i == 2)
            {
                const std::shared_ptr<MotorControllerDependencies> motor_controller_dependencies = motor_controller_dependencies_init(i);
                if (motor_controller_dependencies == nullptr)
                {
                    ESP_LOGE(FUNCTION_NAME, "Failed to create motor controller dependencies");
                    return;
                }
                servos[i] = new MotorController(motor_controller_dependencies);
            }
        }
    }

    ~RobotArm()
    {
        ESP_LOGW(FUNCTION_NAME, "RobotArm destructor called");
        for (auto const &x : servos)
        {
            delete x.second;
        }
    }

    const std::shared_ptr<MotorControllerDependencies> motor_controller_dependencies_init(uint32_t id)
    {
        QueueHandle_t inQ = xQueueCreate(10, sizeof(twai_message_t));
        if (inQ == nullptr)
        {
            ESP_LOGE(FUNCTION_NAME, "Failed to create queue");
            return nullptr;
        }

        std::shared_ptr<TWAICommandFactorySettings> settings = std::make_shared<TWAICommandFactorySettings>(id, inQ, twai_controller->outQ, command_lifecyle_registry);
        std::shared_ptr<TWAICommandFactory> command_factory = std::make_shared<TWAICommandFactory>(settings);
        std::shared_ptr<MotorContext> motor_context = std::make_shared<MotorContext>(id);
        std::shared_ptr<MotorResponseHandler> motor_response_handler = std::make_shared<MotorResponseHandler>(id, motor_context, command_mapper);
        const std::shared_ptr<MotorControllerDependencies> container = std::make_shared<MotorControllerDependencies>(id, inQ, twai_controller->outQ, system_event_group, command_mapper, command_lifecyle_registry, command_factory, motor_context, motor_response_handler);

        return container;
    }
};
#endif // ROBOTARM_HPP