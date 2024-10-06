#ifndef GENERIC_COMMAND_BUILDER_H
#define GENERIC_COMMAND_BUILDER_H

#include "../common/utils.hpp"
#include "TWAICommandBuilderBase.hpp"
#include "TypeDefs.hpp"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/queue.h"
#include <driver/twai.h>
#include <vector>

class GenericCommandBuilder : public TWAICommandBuilderBase<GenericCommandBuilder>
{
private:
    std::variant<std::vector<uint8_t>, std::vector<uint16_t>, std::vector<uint32_t>> payload;

public:
    GenericCommandBuilder(std::shared_ptr<TWAICommandFactorySettings> settings, uint8_t command_code) : TWAICommandBuilderBase<GenericCommandBuilder>(settings, command_code) {}
    GenericCommandBuilder(std::shared_ptr<TWAICommandFactorySettings> settings, uint8_t command_code, std::vector<uint8_t> payload) : TWAICommandBuilderBase<GenericCommandBuilder>(settings, command_code), payload(payload) {}
    GenericCommandBuilder(std::shared_ptr<TWAICommandFactorySettings> settings, uint8_t command_code, std::vector<uint16_t> payload) : TWAICommandBuilderBase<GenericCommandBuilder>(settings, command_code), payload(payload) {}
    GenericCommandBuilder(std::shared_ptr<TWAICommandFactorySettings> settings, uint8_t command_code, std::vector<uint32_t> payload) : TWAICommandBuilderBase<GenericCommandBuilder>(settings, command_code), payload(payload) {}
    ~GenericCommandBuilder()
    {
        ESP_LOGW(FUNCTION_NAME, "GenericCommandBuilder destructor called");
        delete[] data;
    }

    void set_data()
    {
        std::visit([this](auto &&arg)
                   {
            using T = std::decay_t<decltype(arg)>;
            for (size_t i = 0; i < arg.size(); ++i)
            {
                if constexpr (std::is_same_v<T, std::vector<uint8_t>>)
                {
                    data[i + 1] = arg[i];
                }
                else if constexpr (std::is_same_v<T, std::vector<uint16_t>>)
                {
                    data[i + 1] = static_cast<uint8_t>(arg[i] & 0xFF);
                    data[i + 2] = static_cast<uint8_t>((arg[i] >> 8) & 0xFF);
                }
                else if constexpr (std::is_same_v<T, std::vector<uint32_t>>)
                {
                    data[i + 1] = static_cast<uint8_t>(arg[i] & 0xFF);
                    data[i + 2] = static_cast<uint8_t>((arg[i] >> 8) & 0xFF);
                    data[i + 3] = static_cast<uint8_t>((arg[i] >> 16) & 0xFF);
                    data[i + 4] = static_cast<uint8_t>((arg[i] >> 24) & 0xFF);
                }
            } }, payload);
    }

    esp_err_t build_twai_message() override
    {
        data[0] = command_code;
        if (!std::holds_alternative<std::vector<uint8_t>>(payload) && !std::holds_alternative<std::vector<uint16_t>>(payload) && !std::holds_alternative<std::vector<uint32_t>>(payload))
        {
            return ESP_ERR_INVALID_ARG;
        }
        if (std::visit([](auto &&arg)
                       { return arg.size(); }, payload) > 0)
        {
            set_data();
        }
        set_msg_data_crc();
        return ESP_OK;
    }
};
#endif // GENERIC_COMMAND_BUILDER_H