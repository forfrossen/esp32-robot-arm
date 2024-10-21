#ifndef COMMAND_LIFECYCLE_REGISTRY_H
#define COMMAND_LIFECYCLE_REGISTRY_H

#include "../common/utils.hpp"
#include "../magic_enum/include/magic_enum/magic_enum.hpp"
#include "CommandLifecycleFSM.hpp"
#include "esp_err.h"
#include "esp_log.h"
#include <chrono>
#include <functional>
#include <future>
#include <map>

struct CommandRegistryEntry
{
    CommandIds command_id;
    twai_message_t message;
    CommandLifecycleFSM state;
    std::chrono::system_clock::time_point cdate, udate;
    std::future<std::function<void()>> getFuture();
    std::optional<std::any> requested_change;
    std::optional<std::any> response;

    CommandRegistryEntry(CommandIds command_id, twai_message_t message) : command_id(command_id),
                                                                          message(message),
                                                                          cdate(std::chrono::system_clock::now()),
                                                                          udate(std::chrono::system_clock::now())
    {
        ESP_LOGI(FUNCTION_NAME, "MotorCommandLifecycleRegister constructor called");
    };
};

using MotorCommandRegistry = std::map<uint8_t, CommandRegistryEntry>;
using ArmRegistry = std::map<uint32_t, MotorCommandRegistry>;

class CommandLifecycleRegistry
{
public:
    CommandLifecycleRegistry()
    {
        ESP_LOGI(FUNCTION_NAME, "CommandLifecycleRegistry constructor called");
    };

    ~CommandLifecycleRegistry() {}

    esp_err_t register_new_motor(uint32_t motor_id)
    {
        ESP_LOGI(FUNCTION_NAME, "Registering new motor with ID: 0x%lu", motor_id);
        arm_registry[motor_id] = MotorCommandRegistry();
        return ESP_OK;
    };

    esp_err_t get_motor_registry(uint32_t motor_id, MotorCommandRegistry *&motor_registry)
    {
        ESP_LOGI(FUNCTION_NAME, "Getting motor registry for motor 0x%lu", motor_id);
        auto it = arm_registry.find(motor_id);
        CHECK_THAT(it != arm_registry.end());
        motor_registry = &(it->second);
        return ESP_OK;
    };

    esp_err_t register_command(uint32_t motor_id, uint8_t command_id, twai_message_t message)
    {
        esp_err_t ret;
        ESP_LOGI(FUNCTION_NAME, "Registering command for motor 0x%lu with ID: 0x%02X", motor_id, command_id);
        CommandRegistryEntry command = CommandRegistryEntry(static_cast<CommandIds>(command_id), message);
        MotorCommandRegistry *motor_registry;
        ESP_RETURN_ON_ERROR(
            get_motor_registry(motor_id, motor_registry),
            FUNCTION_NAME,
            "Failed to get motor registry for motor 0x%lu", motor_id);
        CommandRegistryEntry *existing_command = nullptr;
        ret = find_command(motor_id, command_id, existing_command);
        if (ret == ESP_OK)
        {
            ESP_LOGW(FUNCTION_NAME, "Command already exists for motor 0x%lu with ID: 0x%02X", motor_id, command_id);
            // Delete existing command and replace it with the new one
            motor_registry->erase(command_id);
            return ESP_FAIL;
        }

        CHECK_THAT(motor_registry->emplace(command_id, command).second);
        return ESP_OK;
    };

    esp_err_t find_command(uint32_t motor_id, uint8_t command_id, CommandRegistryEntry *&command)
    {
        ESP_LOGI(FUNCTION_NAME, "Finding command for motor 0x%lu with ID: 0x%02X", motor_id, command_id);
        MotorCommandRegistry *motor_registry;
        ESP_RETURN_ON_ERROR(
            get_motor_registry(motor_id, motor_registry),
            FUNCTION_NAME,
            "Failed to get motor registry for motor 0x%lu", motor_id);

        auto it = motor_registry->find(command_id);
        CHECK_THAT(it != motor_registry->end());

        command = &(it->second);
        return ESP_OK;
    };

    esp_err_t update_command_state(uint32_t motor_id, uint8_t command_id, std::optional<std::any> data = std::nullopt)
    {
        ESP_LOGI(FUNCTION_NAME, "Updating command state for motor 0x%lu with ID: 0x%02X ", motor_id, command_id);
        CommandRegistryEntry *command;
        CHECK_THAT(find_command(motor_id, command_id, command) == ESP_OK);
        CHECK_THAT(command->state.next_state() == ESP_OK);
        command->response = data;
    }

private:
    ArmRegistry arm_registry;
};
#endif // COMMAND_LIFECYCLE_REGISTRY_H