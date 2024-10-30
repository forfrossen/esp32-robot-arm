#ifndef ROBOTARM_HPP
#define ROBOTARM_HPP

#include "Builder.hpp"
#include "CommandFactory.hpp"
#include "CommandLifecycleRegistry.hpp"
#include "Controller.hpp"
#include "ICommand.h"
#include "SetRunModeCommand.h"
#include "TWAIController.hpp"
#include "TypeDefs.hpp"
#include "freeRTOS/queue.h"
#include "freertos/FreeRTOS.h"
#include "utils.hpp"
#include <memory>

#include "Events.hpp"

typedef struct
{
    bool active;
    std::string name;
} MotorConfig;

using MotorConfigMap = std::map<uint8_t, MotorConfig>;

using MotorRegistry = std::map<uint8_t, std::unique_ptr<MotorController>>;

MotorConfigMap motor_configs = {
    {1, {false, "Shoulder"}},
    {2, {true, "Elbow"}},
    {3, {false, "Wrist"}},
    {4, {false, "Gripper"}},
    {5, {false, "Base"}},
    {6, {false, "Wrist Rotation"}},
};

class RobotArm
{
private:
    esp_event_loop_handle_t system_event_loop;
    EventGroupHandle_t system_event_group;
    std::shared_ptr<TWAIController> twai_controller;
    std::shared_ptr<CommandLifecycleRegistry> command_lifecyle_registry;
    MotorRegistry motors;

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

        command_lifecyle_registry = std::make_shared<CommandLifecycleRegistry>();

        esp_err_t ret = register_event_handlers();
        if (ret != ESP_OK)
        {
            ESP_LOGE(FUNCTION_NAME, "Failed to register Event Handlers for Robot_Arm (System_Events)");
        }

        ret = start_motors();
        if (ret != ESP_OK)
        {
            ESP_LOGE(FUNCTION_NAME, "Failed to start motors");
        }
    }

    ~RobotArm()
    {
        ESP_LOGW(FUNCTION_NAME, "RobotArm destructor called");
    }

    esp_err_t register_event_handlers()
    {
        esp_err_t ret = esp_event_handler_register_with(
            system_event_loop,
            SYSTEM_EVENTS,
            REMOTE_CONTROL_EVENT,
            &on_REMOTE_CONTROL_EVENT,
            this);

        CHECK_THAT(ret == ESP_OK);

        ret = esp_event_handler_register_with(
            system_event_loop,
            SYSTEM_EVENTS,
            SET_RUN_MODE_EVENT,
            &handle_set_run_mode_command,
            this);
        CHECK_THAT(ret == ESP_OK);

        return ESP_OK;
    }

    esp_err_t start_motors()
    {
        if (motors.size() > 0)
        {
            ESP_LOGW(FUNCTION_NAME, "Motors already started");
            return ESP_OK;
        }

        // for each motor in motor_configs
        // create a MotorController
        // add to motors map

        for (auto const &x : motor_configs)
        {
            CONT_IF_CHECK_FAILS(x.first > 0);
            CONT_IF_CHECK_FAILS(x.first <= 6);
            CONT_IF_CHECK_FAILS(x.second.active);
            auto builder = new MotorBuilder(
                x.first,
                twai_controller,
                command_lifecyle_registry,
                system_event_loop,
                system_event_group);

            esp_err_t ret = builder->build_dependencies();
            CHECK_THAT(ret == ESP_OK);

            motors[x.first] = builder->get_result();
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
            ESP_LOGW(FUNCTION_NAME, "Unknown message: %d", magic_enum::enum_integer(evt_msg));
        }
    }

    static void handle_set_run_mode_command(void *args, esp_event_base_t event_base, int32_t event_id, void *event_data)
    {
        ESP_LOGD(FUNCTION_NAME, "handle_set_run_mode_command");
        ESP_RETURN_VOID_ON_FALSE(event_id == SET_RUN_MODE_EVENT, FUNCTION_NAME, "Event id is not SET_RUN_MODE_EVENT");

        auto *self = static_cast<RobotArm *>(args);
        ESP_RETURN_VOID_ON_FALSE(self != nullptr, FUNCTION_NAME, "RobotArm instance is null in system event handler");

        ESP_LOGD(FUNCTION_NAME, "GOT SYSTEM EVENT, EVENT_BASE: %s, EVENT_ID: %ld", event_base, event_id);

        CommandEventData *data = static_cast<CommandEventData *>(event_data);
        if (!data || !data->command)
        {
            ESP_LOGE(FUNCTION_NAME, "Failed to cast event_data to CommandEventData*.");
            delete data;
            delete data->command;
            return;
        }

        SetRunmodeCommand *set_runmode_cmd = static_cast<SetRunmodeCommand *>(data->command);
        if (!set_runmode_cmd)
        {
            ESP_LOGE("HandlerRegistration", "Failed to cast ICommand* to SetRunmodeCommand*.");
            delete data->command;
            delete data;
            return;
        }

        int run_level = set_runmode_cmd->get_run_mode();
        RunMode run_mode = static_cast<RunMode>(run_level);

        if (run_level >= 0 && run_level <= 3)
        {
            ESP_LOGE(FUNCTION_NAME, "Invalid runlevel: %d", run_level);
            delete data->command;
            delete data;
            return;
        }

        switch (run_level)
        {
        case 0:
            xEventGroupSetBits(self->system_event_group, RUNLEVEL_0);
            xEventGroupClearBits(self->system_event_group, RUNLEVEL_1 | RUNLEVEL_2 | RUNLEVEL_3);
            ESP_LOGI(FUNCTION_NAME, "Setting runlevel 0");
            break;
        case 1:
            xEventGroupSetBits(self->system_event_group, RUNLEVEL_1 | RUNLEVEL_0);
            xEventGroupClearBits(self->system_event_group, RUNLEVEL_2 | RUNLEVEL_3);
            ESP_LOGI(FUNCTION_NAME, "Setting runlevel 1");
            break;
        case 2:
            xEventGroupSetBits(self->system_event_group, RUNLEVEL_2 | RUNLEVEL_1 | RUNLEVEL_0);
            xEventGroupClearBits(self->system_event_group, RUNLEVEL_3);
            ESP_LOGI(FUNCTION_NAME, "Setting runlevel 2");
            break;
        case 3:
            xEventGroupSetBits(self->system_event_group, RUNLEVEL_3 | RUNLEVEL_2 | RUNLEVEL_1 | RUNLEVEL_0);
            ESP_LOGI(FUNCTION_NAME, "Setting runlevel 3");
            break;
        default:
            ESP_LOGW(FUNCTION_NAME, "Invalid runlevel: %d", run_level);
        }
        delete set_runmode_cmd;
        delete data->command;
        delete data;
        ESP_LOGD(FUNCTION_NAME, "Run mode: %s", magic_enum::enum_name(run_mode).data());
    }
};
#endif // ROBOTARM_HPP