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
    uint8_t status;
    std::chrono::system_clock::time_point cdate, udate;
    std::future<std::function<void()>> getFuture();
    std::optional<std::any> requested_change;
    std::optional<std::any> response;

    std::map<std::string, MotorPropertyVariant> properties_requested;

    CommandRegistryEntry(CommandIds command_id, twai_message_t message) : command_id(command_id),
                                                                          message(message),
                                                                          cdate(std::chrono::system_clock::now()),
                                                                          udate(std::chrono::system_clock::now())
    {
        ESP_LOGI(FUNCTION_NAME, "CommandRegistryEntry constructor called");
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

    ~CommandLifecycleRegistry()
    {
        ESP_LOGE(FUNCTION_NAME, "CommandLifecycleRegistry destructor called");
    }

    esp_err_t register_new_motor(uint32_t motor_id)
    {
        ESP_LOGI(FUNCTION_NAME, "Registering new motor with ID: 0x0%lu", motor_id);
        arm_registry.emplace(motor_id, MotorCommandRegistry());
        return ESP_OK;
    };

    esp_err_t get_motor_registry(uint32_t motor_id, MotorCommandRegistry *&motor_registry)
    {
        ESP_LOGI(FUNCTION_NAME, "Getting motor registry for motor 0x0%lu", motor_id);
        auto it = arm_registry.find(motor_id);
        CHECK_THAT(it != arm_registry.end());
        motor_registry = &(it->second);
        return ESP_OK;
    };

    esp_err_t find_command(MotorCommandRegistry *motor_registry, uint8_t command_id, CommandRegistryEntry *&command)
    {
        auto it = motor_registry->find(command_id);
        if (it == motor_registry->end())
        {
            ESP_LOGI(FUNCTION_NAME, "Command with ID: 0x%02X not found", command_id);
            return ESP_FAIL;
        }
        ESP_LOGI(FUNCTION_NAME, "Finding command with ID: 0x%02X", command_id);
        command = &(it->second);
        return ESP_OK;
    };

    esp_err_t update_command_state(uint32_t motor_id, uint8_t command_id, std::optional<uint8_t *> data = std::nullopt)
    {
        ESP_LOGI(FUNCTION_NAME, "Updating command state for motor 0x0%lu with ID: 0x%02X ", motor_id, command_id);
        MotorCommandRegistry *motor_registry;
        CommandRegistryEntry *command;
        CHECK_THAT(get_motor_registry(motor_id, motor_registry) == ESP_OK);
        CHECK_THAT(find_command(motor_registry, command_id, command) == ESP_OK);
        CHECK_THAT(command->state.next_state() == ESP_OK);
        if (!data.has_value())
        {
            return ESP_OK;
        }

        if (IS_STATUS_IN_DATA1(command_id) && data.has_value())
        {
            command->status = (*data)[1];
        }
        return ESP_OK;
    }

    esp_err_t register_command_error(uint32_t motor_id, uint8_t command_id)
    {
        ESP_LOGI(FUNCTION_NAME, "Registering command error for motor 0x0%lu with ID: 0x%02X ", motor_id, command_id);
        MotorCommandRegistry *motor_registry;
        CommandRegistryEntry *command;
        CHECK_THAT(get_motor_registry(motor_id, motor_registry) == ESP_OK);
        CHECK_THAT(find_command(motor_registry, command_id, command) == ESP_OK);
        CHECK_THAT(command->state.transition_to_error() == ESP_OK);
    }

    esp_err_t register_command(uint32_t motor_id, uint8_t command_id, twai_message_t message)
    {
        esp_err_t ret;
        ESP_LOGI(FUNCTION_NAME, "Registering commandId: 0x%02X for motor 0x0%lu ", command_id, motor_id);
        CommandRegistryEntry command = CommandRegistryEntry(static_cast<CommandIds>(command_id), message);
        MotorCommandRegistry *motor_registry;
        ESP_RETURN_ON_ERROR(
            get_motor_registry(motor_id, motor_registry),
            FUNCTION_NAME,
            "Failed to get motor registry for motor 0x%lu", motor_id);

        CommandRegistryEntry *existing_command = nullptr;
        ret = find_command(motor_registry, command_id, existing_command);
        if (ret == ESP_OK)
        {
            ESP_LOGW(FUNCTION_NAME, "Command with ID: 0x%02X already exists for motor 0x0%lu ", command_id, motor_id);
            // // Delete existing command and replace it with the new one
            motor_registry->erase(command_id);

            // RETURN FALSE IF COMMAND ALREADY EXISTS
            // return ESP_FAIL;
        }
        ESP_LOGI(FUNCTION_NAME, "Command with ID: 0x%02X does not exist for motor 0x0%lu. Registering it.", command_id, motor_id);

        CHECK_THAT(motor_registry->emplace(command_id, command).second);
        return ESP_OK;
    };

private:
    ArmRegistry arm_registry;
};
#endif // COMMAND_LIFECYCLE_REGISTRY_H