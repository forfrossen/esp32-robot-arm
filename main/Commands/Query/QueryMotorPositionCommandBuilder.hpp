#ifndef QUERY_MOTOR_POSITION_COMMAND_BUILDER_HPP
#define QUERY_MOTOR_POSITION_COMMAND_BUILDER_HPP

#include "..\MotorController.hpp"
#include "Command.hpp"
#include "TWAICommandBuilderBase.hpp"
#include "esp_err.h"
#include "esp_log.h"

class QueryMotorPositionCommandBuilder : public TWAICommandBuilderBase
{

public:
    QueryMotorPositionCommandBuilder(uint32_t id, QueueHandle_t outQ) : TWAICommandBuilderBase(id, outQ)
    {
        msg.data_length_code = 2;
    }

    esp_err_t build_twai_message_and_enqueue()
    {
        uint8_t data[2] = {0x30, 0x00};

        ESP_LOGI(FUNCTION_NAME, "Querying motor position");

        finalize_message(data);

        esp_err_t ret = enqueueMessage();
        return ret;
    }
};

#endif // QUERY_MOTOR_POSITION_COMMAND_BUILDER_HPP