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
    GenericCommandBuilder(TWAICommandFactorySettings settings) : TWAICommandBuilderBase<GenericCommandBuilder>(settings)
    {
        ESP_LOGI(FUNCTION_NAME, "GenericCommandBuilder constructor called");
    }

    GenericCommandBuilder &init_new_command(uint8_t cmd_code)
    {
        set_command_code(cmd_code);
        set_data_length_code(2);
        create_msg_data();
        return *this;
    }

    GenericCommandBuilder &init_new_command(uint8_t cmd_code, std::vector<uint8_t> payload)
    {
        set_command_code(cmd_code);
        payload = payload;
        set_data_length_code(2 + payload.size());
        create_msg_data();
        return *this;
    }

    // (uint8_t cmd_code, std::vector<uint8_t> payload) override
    // {
    //     set_command_code(cmd_code);
    //     set_payload(payload);
    //     set_data_length_code();
    //     return *this;
    // }

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