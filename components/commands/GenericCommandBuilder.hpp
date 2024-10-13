#ifndef GENERIC_COMMAND_BUILDER_H
#define GENERIC_COMMAND_BUILDER_H

#include "../common/utils.hpp"
#include "Events.hpp"
#include "TWAICommandBuilderBase.hpp"
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

class GenericCommandBuilder : public TWAICommandBuilderBase<GenericCommandBuilder>
{
private:
public:
    GenericCommandBuilder(std::shared_ptr<TWAICommandFactorySettings> settings, CommandIds command_code) : TWAICommandBuilderBase<GenericCommandBuilder>(settings, command_code) {}

    ~GenericCommandBuilder()
    {
        ESP_LOGW(FUNCTION_NAME, "GenericCommandBuilder destructor called");
        delete[] data;
        vSemaphoreDelete(msg_mutex);
    }

    template <typename... Args>
    void with(Args... args)
    {
        static_assert((... && (std::is_same_v<Args, uint8_t> ||
                               std::is_same_v<Args, uint16_t> ||
                               std::is_same_v<Args, uint32_t>)),
                      "Arguments must be of type uint8_t, uint16_t, or uint32_t");

        size_t index = 0;
        ([&]
         {
        if constexpr (std::is_same_v<Args, uint8_t>)
        {
            if (index < 8) data[index++] = args;
        }
        else if constexpr (std::is_same_v<Args, uint16_t>)
        {
            if (index < 8) data[index++] = static_cast<uint8_t>(args & 0xFF);
            if (index < 8) data[index++] = static_cast<uint8_t>((args >> 8) & 0xFF);
        }
        else if constexpr (std::is_same_v<Args, uint32_t>)
        {
            if (index < 8) data[index++] = static_cast<uint8_t>(args & 0xFF);
            if (index < 8) data[index++] = static_cast<uint8_t>((args >> 8) & 0xFF);
            if (index < 8) data[index++] = static_cast<uint8_t>((args >> 16) & 0xFF);
            if (index < 8) data[index++] = static_cast<uint8_t>((args >> 24) & 0xFF);
        } }(), ...);
    }

    esp_err_t build_twai_message() override
    {
        ESP_LOGI(FUNCTION_NAME, "Building TWAI message for command: %s", GET_CMD(command_code));
        return ESP_OK;
    }
};
#endif // GENERIC_COMMAND_BUILDER_H