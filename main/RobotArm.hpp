#ifndef ROBOTARM_HPP
#define ROBOTARM_HPP

#include "Builder.hpp"
#include "CommandFactory.hpp"
#include "CommandLifecycleRegistry.hpp"
#include "Controller.hpp"
#include "IWsCommand.h"
#include "SetRunLevelCommand.h"
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
    RunLevel run_level;

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
            &on_rpc_set_run_level,
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

    static void on_rpc_set_run_level(void *args, esp_event_base_t event_base, int32_t event_id, void *event_data)
    {
        auto *instance = static_cast<RobotArm *>(args);
        ESP_RETURN_VOID_ON_FALSE(instance != nullptr, FUNCTION_NAME, "RobotArm instance is null in system event handler");

        ESP_LOGD(FUNCTION_NAME, "on_rpc_set_run_level");
        ESP_RETURN_VOID_ON_FALSE(event_id == SET_RUN_MODE_EVENT, FUNCTION_NAME, "Event id is not SET_RUN_MODE_EVENT");
        ESP_LOGD(FUNCTION_NAME, "GOT SYSTEM EVENT, EVENT_BASE: %s, EVENT_ID: %ld", event_base, event_id);

        auto *data = static_cast<rpc_event_data *>(event_data);
        ESP_RETURN_VOID_ON_FALSE(data != nullptr, FUNCTION_NAME, "rpc_event_data is null in system event handler");
        ESP_RETURN_VOID_ON_FALSE(data->command != nullptr, FUNCTION_NAME, "rpc_event_data command is null in system event handler");
        auto *rpc_cmd = static_cast<SetRunLevelCommand *>(data->command);
        esp_err_t ret = instance->set_runlevel(rpc_cmd);
        if (ret != ESP_OK)
        {
            ESP_LOGE(FUNCTION_NAME, "Failed to set runlevel");
            rpc_cmd->post_result(false);
        }
        else
        {
            ESP_LOGD(FUNCTION_NAME, "Runlevel set. Posting positive result");
            rpc_cmd->post_result(true);
        }
    }

    esp_err_t set_runlevel(SetRunLevelCommand *rpc_cmd)
    {
        ESP_RETURN_ON_FALSE(rpc_cmd != nullptr, ESP_ERR_INVALID_ARG, FUNCTION_NAME, "Failed to cast IWsCommand* to SetRunLevelCommand*.");

        run_level = rpc_cmd->get_run_level();
        auto set_runlevel = [&](uint32_t set_bits, uint32_t clear_bits)
        {
            xEventGroupSetBits(system_event_group, set_bits);
            xEventGroupClearBits(system_event_group, clear_bits);
        };

        switch (run_level)
        {
        case RunLevel::RUNMODE0:
            set_runlevel(RUNLEVEL_0, RUNLEVEL_1 | RUNLEVEL_2 | RUNLEVEL_3);
            ESP_LOGI(FUNCTION_NAME, "Setting runlevel 0");
            break;
        case RunLevel::RUNMODE1:
            set_runlevel(RUNLEVEL_1 | RUNLEVEL_0, RUNLEVEL_2 | RUNLEVEL_3);
            ESP_LOGI(FUNCTION_NAME, "Setting runlevel 1");
            break;
        case RunLevel::RUNMODE2:
            set_runlevel(RUNLEVEL_2 | RUNLEVEL_1 | RUNLEVEL_0, RUNLEVEL_3);
            ESP_LOGI(FUNCTION_NAME, "Setting runlevel 2");
            break;
        case RunLevel::RUNMODE3:
            set_runlevel(RUNLEVEL_3 | RUNLEVEL_2 | RUNLEVEL_1 | RUNLEVEL_0, 0);
            ESP_LOGI(FUNCTION_NAME, "Setting runlevel 3");
            break;
        default:
            ESP_LOGW(FUNCTION_NAME, "Invalid runlevel: %d", static_cast<int>(run_level));
            return ESP_ERR_INVALID_ARG;
        }

        return ESP_OK;
    }
};
#endif // ROBOTARM_HPP