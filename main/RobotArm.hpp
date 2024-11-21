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
#include "utils.hpp"
#include <freertos/FreeRTOS.h>
#include <memory>
#include <variant>

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

        rpc_event_data evt_data = *reinterpret_cast<rpc_event_data *>(event_data);
        auto ws_command = evt_data.command;
        auto ws_msg = ws_command->get_msg();
        auto cmd = ws_command->get_cmd_enum();

        ESP_LOGD(FUNCTION_NAME, "Remote Control Event with command  %s", cmd);
        ESP_LOGD(FUNCTION_NAME, "Remote Control Event for command data %s", ws_msg.params.dump().c_str());

        ESP_RETURN_VOID_ON_FALSE(
            std::holds_alternative<motor_command_id_t>(ws_msg.command),
            FUNCTION_NAME,
            "Command is not a motor command");

        ESP_LOGW(FUNCTION_NAME, "Command is not a motor command");
        auto cmd_id = std::get<motor_command_id_t>(ws_msg.command);
        ESP_LOGD(FUNCTION_NAME, "Command ID: %d", cmd_id);

        auto cmd_string = magic_enum::enum_name(cmd_id).data();
        ESP_LOGD(FUNCTION_NAME, "Command String: %s", cmd_string);

        auto params = ws_msg.params;
        ESP_LOGD(FUNCTION_NAME, "Params: %s", params.dump().c_str());

        auto id = ws_command->get_motor_id();
        auto position = params["position"].get<int32_t>();
        auto speed = params["speed"].get<int32_t>();
        auto acceleration = params["acceleration"].get<int32_t>();
        auto direction = params["direction"].get<bool>();

        auto ret = robot_arm->motors[id]->set_target_position(position, speed, acceleration, direction);
        if (ret == ESP_OK)
        {
            ESP_LOGI(FUNCTION_NAME, "Motor %d set to position %lx", id, position);
            ws_command->post_result(true);
        }
        else
        {
            ESP_LOGE(FUNCTION_NAME, "Failed to set motor %d to position %lx", id, position);
            ws_command->post_result(false);
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

        auto run_level = rpc_cmd->get_run_level();
        auto set_runlevel = [&](uint32_t set_bits, uint32_t clear_bits)
        {
            xEventGroupSetBits(system_event_group, set_bits);
            xEventGroupClearBits(system_event_group, clear_bits);
        };

        switch (run_level)
        {
        case RunLevel::RUNLEVEL0:
            set_runlevel(RUNLEVEL_0, RUNLEVEL_1 | RUNLEVEL_2 | RUNLEVEL_3);
            ESP_LOGI(FUNCTION_NAME, "Setting runlevel 0");
            break;
        case RunLevel::RUNLEVEL1:
            set_runlevel(RUNLEVEL_1 | RUNLEVEL_0, RUNLEVEL_2 | RUNLEVEL_3);
            ESP_LOGI(FUNCTION_NAME, "Setting runlevel 1");
            break;
        case RunLevel::RUNLEVEL2:
            set_runlevel(RUNLEVEL_2 | RUNLEVEL_1 | RUNLEVEL_0, RUNLEVEL_3);
            ESP_LOGI(FUNCTION_NAME, "Setting runlevel 2");
            break;
        case RunLevel::RUNLEVEL3:
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