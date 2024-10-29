#ifndef ROBOTARM_HPP
#define ROBOTARM_HPP

#include "Builder.hpp"
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
    std::shared_ptr<TWAIController> twai_controller;
    std::shared_ptr<CommandLifecycleRegistry> command_lifecyle_registry;
    std::map<uint8_t, std::unique_ptr<MotorController>> servos;

public:
    RobotArm(
        esp_event_loop_handle_t system_event_loop,
        EventGroupHandle_t system_event_group)
        : system_event_loop(system_event_loop),
          system_event_group(system_event_group)
    {
        ESP_LOGD(FUNCTION_NAME, "RobotArm constructor called");
        twai_controller = std::make_shared<TWAIController>(system_event_loop, system_event_group);

        ESP_LOGD(FUNCTION_NAME, "Registering for system_event_loop %p handler for REMOTE_CONTROL_EVENT", system_event_loop);

        assert(system_event_loop != nullptr);

        esp_err_t ret = esp_event_handler_register_with(
            system_event_loop,
            SYSTEM_EVENTS,
            REMOTE_CONTROL_EVENT,
            on_REMOTE_CONTROL_EVENT,
            this);

        assert(ret == ESP_OK);

        command_lifecyle_registry = std::make_shared<CommandLifecycleRegistry>();

        ESP_LOGD(FUNCTION_NAME, "Posting ARM_INITIALIZING event");
        ret = esp_event_post_to(
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

        start_motors();
    }

    ~RobotArm()
    {
        ESP_LOGW(FUNCTION_NAME, "RobotArm destructor called");
    }

    esp_err_t start_motors()
    {
        if (servos.size() > 0)
        {
            ESP_LOGW(FUNCTION_NAME, "Motors already started");
            return ESP_OK;
        }

        ESP_LOGD(FUNCTION_NAME, "Starting motors");
        for (int i = 1; i < 4; i++)
        {
            if (i == 2)
            {
                // const std::shared_ptr<MotorControllerDependencies> motor_controller_dependencies = motor_controller_dependencies_init(i);
                // ESP_RETURN_ON_FALSE(
                //     motor_controller_dependencies,
                //     ESP_FAIL,
                //     FUNCTION_NAME,
                //     "Failed to create motor controller dependencies for motor %d", i);
                auto builder = new MotorBuilder(
                    i,
                    twai_controller,
                    command_lifecyle_registry,
                    system_event_loop,
                    system_event_group);

                esp_err_t ret = builder->build_dependencies();
                CHECK_THAT(ret == ESP_OK);

                servos[i] = builder->get_result();
            }
        }
        return ESP_OK;
    }

    static void on_REMOTE_CONTROL_EVENT(void *args, esp_event_base_t event_base, int32_t event_id, void *event_data)
    {

        ESP_RETURN_VOID_ON_FALSE(event_id == REMOTE_CONTROL_EVENT, FUNCTION_NAME, "Event id is not REMOTE_CONTROL_EVENT");
        RobotArm *robot_arm = static_cast<RobotArm *>(args);
        ESP_RETURN_VOID_ON_FALSE(robot_arm != nullptr, FUNCTION_NAME, "RobotArm instance is null in system event handler");
        ESP_LOGD(FUNCTION_NAME, "GOT SYSTEM EVENT, EVENT_BASE: %s, EVENT_ID: %ld", event_base, event_id);

        remote_control_event_t evt_msg = *reinterpret_cast<remote_control_event_t *>(event_data);

        ESP_LOGD(FUNCTION_NAME, "Message: %s", magic_enum::enum_name(evt_msg).data());

        if (evt_msg == remote_control_event_t::START_MOTORS)
        {
            ESP_LOGI(FUNCTION_NAME, "Starting motors");
            robot_arm->start_motors();
        }
        else if (evt_msg == remote_control_event_t::STOP_MOTORS)
        {
            ESP_LOGI(FUNCTION_NAME, "Stopping motors");
            // for (auto const &x : robot_arm->servos)
            // {
            //     x.second->~MotorController();
            // }
        }
        else
        {
            ESP_LOGW(FUNCTION_NAME, "Unknown message: %d", evt_msg);
        }
    }
};
#endif // ROBOTARM_HPP