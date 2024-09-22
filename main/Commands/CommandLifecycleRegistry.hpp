#ifndef COMMAND_LIFECYCLE_REGISTRY_H
#define COMMAND_LIFECYCLE_REGISTRY_H

#include "CommandLifecycleFSM.hpp"
#include "esp_err.h"
#include "esp_log.h"
#include "utils.hpp"
#include <functional>
#include <map>

struct MotorCommandLifecycleRegister
{
    uint8_t command_id;
    CommandLifecycleFSM state;
};

struct ArmCommandLifecycleRegister
{
    uint32_t motor_id;
    MotorCommandLifecycleRegister motor_commands;
};

class CommandLifecycleRegistry
{
public:
    CommandLifecycleRegistry()
    {
        ESP_LOGI(FUNCTION_NAME, "CommandLifecycleRegistry constructor called");
        arm_commands = {};
    };
    ~CommandLifecycleRegistry() {}

    void register_new_motor(uint8_t motor_id)
    {
        ESP_LOGI(FUNCTION_NAME, "Registering new motor with ID: 0x%02X", motor_id);
        arm_commands.motor_id = motor_id;
    };

    void register_command(uint32_t id, uint8_t command_id)
    {
        ESP_LOGI(FUNCTION_NAME, "Registering command with ID: 0x%lu", id);
        arm_commands.motor_commands.command_id = command_id;
    };

private:
    ArmCommandLifecycleRegister arm_commands;
};
#endif // COMMAND_LIFECYCLE_REGISTRY_H