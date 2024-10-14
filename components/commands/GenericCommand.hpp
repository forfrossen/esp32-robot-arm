#ifndef GENERIC_COMMAND_BUILDER_H
#define GENERIC_COMMAND_BUILDER_H

#include "../common/utils.hpp"
#include "CommandBase.hpp"
#include "Events.hpp"
#include "TypeDefs.hpp"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/queue.h"
#include <driver/twai.h>
#include <variant>
#include <vector>

// Command Argument Types (for input arguments)
using CommandArg = std::variant<uint8_t, uint16_t, float, int32_t, bool>;

// Command Return Types (for output/return values)
using CommandReturn = std::variant<std::monostate, uint8_t, uint16_t, float, int32_t, bool>; // std::monostate is for void return types

// Command Signature: Defines the argument type and the return type for each command
struct CommandSignature
{
    std::function<CommandReturn(CommandArg)> command_handler; // A function that takes a CommandArg and returns CommandReturn
};

class GenericCommand : public CommandBase<GenericCommand>
{
private:
public:
    GenericCommand(std::shared_ptr<CommandFactorySettings> settings, CommandIds command_code) : CommandBase<GenericCommand>(settings, command_code) {}

    ~GenericCommand()
    {
        ESP_LOGW(FUNCTION_NAME, "GenericCommand destructor called");
        vSemaphoreDelete(msg_mutex);
    }

    template <typename... Args>
    esp_err_t with(Args... args)
    {
        CHECK_THAT(data != nullptr);
        // Ensure no more data is provided than the array can handle
        CHECK_THAT(sizeof...(args) <= data_length);

        // Ensure that all arguments are of valid types (uint8_t, uint16_t, or uint32_t)
        static_assert((... && (std::is_same_v<Args, uint8_t> ||
                               std::is_same_v<Args, uint16_t> ||
                               std::is_same_v<Args, uint24_t> ||
                               std::is_same_v<Args, uint32_t>)),
                      "Arguments must be of type uint8_t, uint16_t, uint24_t or uint32_t");

        size_t index = 0;

        // Lambda to handle each argument type and fill the array
        ([&]
         {
            if constexpr (std::is_same_v<Args, uint8_t>) {
                if (index < data_length) data[index++] = args;  // Directly access the C-style array
            }
            else if constexpr (std::is_same_v<Args, uint16_t>) {
                if (index < data_length) data[index++] = static_cast<uint8_t>(args & 0xFF);         // Lower byte
                if (index < data_length) data[index++] = static_cast<uint8_t>((args >> 8) & 0xFF);  // Upper byte
            }
            else if constexpr (std::is_same_v<Args, uint24_t>) {
                if (index < data_length) data[index++] = static_cast<uint8_t>(args & 0xFF);  // Byte 1 (lowest)
                if (index < data_length) data[index++] = static_cast<uint8_t>((args >> 8) & 0xFF);  // Byte 2 (middle)
                if (index < data_length) data[index++] = static_cast<uint8_t>((args >> 16) & 0xFF);  // Byte 3 (highest
            }
            else if constexpr (std::is_same_v<Args, uint32_t>) {
                if (index < data_length) data[index++] = static_cast<uint8_t>(args & 0xFF);         // Byte 1
                if (index < data_length) data[index++] = static_cast<uint8_t>((args >> 8) & 0xFF);  // Byte 2
                if (index < data_length) data[index++] = static_cast<uint8_t>((args >> 16) & 0xFF);  // Byte 3
                if (index < data_length) data[index++] = static_cast<uint8_t>((args >> 24) & 0xFF);  // Byte 4
            } }(), ...);
        return ESP_OK;
    }

    esp_err_t build_twai_message() override
    {
        ESP_LOGI(FUNCTION_NAME, "Building TWAI message for command");
        CHECK_THAT(data != nullptr);
        ESP_RETURN_ON_ERROR(get_semaphore(), FUNCTION_NAME, "Failed to take mutex");
        // ESP_LOGI(FUNCTION_NAME, "Building TWAI message for command: %s", GET_CMD(command_code));

        ESP_LOGI(FUNCTION_NAME, "Setting command code to: 0x%02X", cmd_code);
        data[0] = static_cast<uint8_t>(command_code);

        xSemaphoreGive(msg_mutex);
        return ESP_OK;
    }
};
#endif // GENERIC_COMMAND_BUILDER_H