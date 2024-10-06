#ifndef COMMAND_LIFECYCLE_REGISTRY_H
#define COMMAND_LIFECYCLE_REGISTRY_H

#include "../common/utils.hpp"
#include "CommandLifecycleFSM.hpp"
#include "esp_err.h"
#include "esp_log.h"
#include <chrono>
#include <functional>
#include <map>

struct StateHistory
{
    CommandLifecycleFSM state;
    std::chrono::time_point<std::chrono::system_clock> udate;
};

struct CommandEntry
{
    CommandLifecycleFSM state;
    std::vector<StateHistory> state_history;
    std::chrono::system_clock::time_point cdate, udate;

    CommandEntry() : cdate(std::chrono::system_clock::now()), udate(std::chrono::system_clock::now())
    {
        ESP_LOGI(FUNCTION_NAME, "MotorCommandLifecycleRegister constructor called");
        state_history.push_back({state, udate});
    };
};

using MotorCommands = std::map<uint8_t, CommandEntry>;

class CommandLifecycleRegistry
{
public:
    CommandLifecycleRegistry()
    {
        ESP_LOGI(FUNCTION_NAME, "CommandLifecycleRegistry constructor called");
    };

    ~CommandLifecycleRegistry() {}

    void register_new_motor(uint32_t motor_id)
    {
        ESP_LOGI(FUNCTION_NAME, "Registering new motor with ID: 0x%lu", motor_id);
        arm_commands[motor_id] = MotorCommands();
    };

    void register_command(uint32_t motor_id, uint8_t command_id)
    {
        ESP_LOGI(FUNCTION_NAME, "Registering command for motor 0x%lu with ID: 0x%02X", motor_id, command_id);
        CommandEntry command;
        if (arm_commands[motor_id].find(command_id) != arm_commands[motor_id].end())
        {
            // Kommando bereits registriert, Ins archiv verschieben oder so
            // commandHistory.push_back(commandRegistry[commandID]);
            arm_commands[motor_id].erase(command_id);
        }

        arm_commands[motor_id].emplace(command_id, CommandEntry());
    };

    void update_command_state(uint32_t motor_id, uint8_t command_id, CommandLifecycleFSM state)
    {
        ESP_LOGI(FUNCTION_NAME, "Updating command state for motor 0x%lu with ID: 0x%02X ", motor_id, command_id);
        if (arm_commands[motor_id].find(command_id) != arm_commands[motor_id].end())
        {
            arm_commands[motor_id][command_id].state = state;
            arm_commands[motor_id][command_id].state_history.push_back({state, std::chrono::system_clock::now()});
        }
    };

private:
    std::map<uint32_t, MotorCommands> arm_commands;
};
#endif // COMMAND_LIFECYCLE_REGISTRY_H