#ifndef GENERIC_COMMAND_BUILDER_H
#define GENERIC_COMMAND_BUILDER_H

#include "TWAICommandBuilderBase.hpp"
#include "TypeDefs.hpp"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/queue.h"
#include "utils.hpp"
#include <driver/twai.h>
#include <vector>

class GenericCommandBuilder : public TWAICommandBuilderBase<GenericCommandBuilder>
{
private:
    std::vector<uint8_t> payload;

public:
    GenericCommandBuilder(std::shared_ptr<TWAICommandFactorySettings> settings, uint8_t command_code) : TWAICommandBuilderBase<GenericCommandBuilder>(settings, command_code) {}
    GenericCommandBuilder(std::shared_ptr<TWAICommandFactorySettings> settings, uint8_t command_code, std::vector<uint8_t> payload) : TWAICommandBuilderBase<GenericCommandBuilder>(settings, command_code, payload) {}
    ~GenericCommandBuilder()
    {
        ESP_LOGW(FUNCTION_NAME, "GenericCommandBuilder destructor called");
        delete[] data;
    }

    void set_data()
    {
        for (int i = 0; i < payload.size(); i++)
        {
            data[i + 1] = payload[i];
        }
    }

    esp_err_t build_twai_message() override
    {
        data[0] = command_code;
        if (payload.size() > 0)
        {
            set_data();
        }
        set_msg_data_crc();
        return ESP_OK;
    }
};
#endif // GENERIC_COMMAND_BUILDER_H