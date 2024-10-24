#ifndef ROBOTARM_HPP
#define ROBOTARM_HPP

#include "CommandFactory.hpp"
#include "CommandLifecycleRegistry.hpp"
#include "Controller.hpp"
#include "TWAIController.hpp"
#include "TypeDefs.hpp"
#include "freeRTOS/queue.h"
#include "freertos/FreeRTOS.h"
#include <memory>

#include "Events.hpp"
class RobotArm
{
private:
    esp_event_loop_handle_t system_event_loop;
    EventGroupHandle_t system_event_group;
    std::map<uint8_t, MotorController *> servos;
    std::shared_ptr<TWAIController> twai_controller;
    std::shared_ptr<CommandLifecycleRegistry> command_lifecyle_registry = std::make_shared<CommandLifecycleRegistry>();

public:
    RobotArm(esp_event_loop_handle_t system_event_loop) : system_event_loop(system_event_loop)
    {
        ESP_LOGI(FUNCTION_NAME, "RobotArm constructor called");
        system_event_group = xEventGroupCreate();

        if (system_event_group == NULL)
        {
            ESP_LOGE(FUNCTION_NAME, "Error creating event group");
        }

        esp_err_t ret = esp_event_post_to(
            system_event_loop,
            SYSTEM_EVENTS,
            ARM_INITIALIZING,
            NULL,
            0,
            portMAX_DELAY);

        if (ret != ESP_OK)
        {
            ESP_LOGE(FUNCTION_NAME, "Failed to post ARM_INITIALIZING event");
        }

        twai_controller = std::make_shared<TWAIController>(system_event_loop);

        for (int i = 1; i < 4; i++)
        {
            if (i == 2)
            {
                const std::shared_ptr<MotorControllerDependencies> motor_controller_dependencies = motor_controller_dependencies_init(i);
                if (motor_controller_dependencies == nullptr)
                {
                    ESP_LOGE(FUNCTION_NAME, "Failed to create motor controller dependencies for motor %d", i);
                    continue;
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
        // Löschen des system_event_loop
        ESP_ERROR_CHECK(esp_event_loop_delete(system_event_loop));
        // Löschen des system_event_group
        vEventGroupDelete(system_event_group);
    }

    const std::shared_ptr<MotorControllerDependencies> motor_controller_dependencies_init(uint32_t id)
    {
        esp_err_t ret = ESP_OK;
        std::string task_name_str = "motor_event_task_" + std::to_string(id);
        esp_event_loop_handle_t motor_event_loop;
        esp_event_loop_args_t motor_loop_args = {
            .queue_size = 5,
            .task_name = task_name_str.c_str(),
            .task_priority = 5,
            .task_stack_size = 1024 * 3,
            .task_core_id = tskNO_AFFINITY};

        ret = esp_event_loop_create(&motor_loop_args, &motor_event_loop);
        GOTO_ON_ERROR(err, "Failed to create motor event loop for motorId: %lu", id);
        ret = twai_controller->register_motor_id(id, motor_event_loop);
        GOTO_ON_ERROR(err, "Failed to register motorId %lu at twai_controller", id);
        ret = command_lifecyle_registry->register_new_motor(id);
        GOTO_ON_ERROR(err, "Failed register motorId %lu at command_lifecyle_registry", id);
    err:
        if (ret != ESP_OK)
        {
            ESP_LOGE(FUNCTION_NAME, "Error initializing motor controller dependencies for motor %lu", id);
            ESP_ERROR_CHECK(esp_event_loop_delete(motor_event_loop));
            return nullptr;
        }

        std::shared_ptr<CommandFactorySettings> settings = std::make_shared<CommandFactorySettings>(id, system_event_loop, command_lifecyle_registry);
        std::shared_ptr<CommandFactory> command_factory = std::make_shared<CommandFactory>(settings);

        // ESP_ERROR_CHECK(esp_event_loop_create(&motor_loop_args, &motor_event_loop));

        std::shared_ptr<event_loops_t> event_loops = std::make_shared<EventLoops>(system_event_loop, motor_event_loop);

        EventGroupHandle_t motor_event_group = xEventGroupCreate();
        if (motor_event_group == NULL)
        {
            ESP_LOGE(FUNCTION_NAME, "Failed to create motor event group for motor %lu", id);
            return nullptr;
        }
        std::shared_ptr<event_groups_t> event_groups = std::make_shared<EventGroups>(system_event_group, motor_event_group);

        std::shared_ptr<MotorContext> motor_context = std::make_shared<MotorContext>(id, event_loops);
        std::shared_ptr<ResponseHandler> motor_response_handler = std::make_shared<ResponseHandler>(id, motor_context, event_loops, command_lifecyle_registry);

        SemaphoreHandle_t motor_mutex = xSemaphoreCreateMutex();
        if (motor_mutex == NULL)
        {
            ESP_LOGE(FUNCTION_NAME, "Failed to create motor mutex for motor %lu", id);
            return nullptr;
        }
        const std::shared_ptr<MotorControllerDependencies> container = std::make_shared<MotorControllerDependencies>(
            id, motor_mutex, event_groups, event_loops, command_lifecyle_registry, command_factory, motor_context, motor_response_handler);
        return container;
    }
};
#endif // ROBOTARM_HPP