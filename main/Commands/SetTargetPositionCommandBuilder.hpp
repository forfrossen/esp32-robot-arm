#ifndef SET_TARGET_POSITION_COMMAND_BUILDER_H
#define SET_TARGET_POSITION_COMMAND_BUILDER_H

#include "..\MotorController.hpp"
#include "Command.hpp"
#include "TWAICommandBuilderBase.hpp"
#include "esp_log.h"

class SetTargetPositionCommandBuilder : public TWAICommandBuilderBase
{
private:
    uint32_t position;
    uint8_t speed;
    uint8_t acceleration;
    bool absolute;

public:
    SetTargetPositionCommandBuilder(uint32_t id, QueueHandle_t outQ) : TWAICommandBuilderBase(id, outQ)
    {
        msg.data_length_code = 8;
    }

    SetTargetPositionCommandBuilder &set_position(uint32_t position)
    {
        this->position = position;
        return *this;
    }

    SetTargetPositionCommandBuilder &set_speed(uint8_t speed)
    {
        this->speed = speed;
        return *this;
    }

    SetTargetPositionCommandBuilder &set_acceleration(uint8_t acceleration)
    {
        this->acceleration = acceleration;
        return *this;
    }

    SetTargetPositionCommandBuilder &set_absolute(bool absolute)
    {
        this->absolute = absolute;
        return *this;
    }

    esp_err_t build_twai_message_and_enqueue()
    {
        uint8_t data[7];
        data[0] = absolute ? 0xF5 : 0xF4;  // Befehlscode für Position mode4: absolute motion by axis
        data[1] = (speed >> 8) & 0x7F;     // Combine direction bit with the upper 7 bits of speed
        data[2] = speed & 0xFF;            // Lower 8 bits of speed
        data[3] = acceleration;            // Beschleunigung
        data[4] = (position >> 16) & 0xFF; // Obere 8 Bits der Position
        data[5] = (position >> 8) & 0xFF;  // Mittlere 8 Bits der Position
        data[6] = position & 0xFF;         // Untere 8 Bits der Position
                                           // data[7] = calculateCRC(data, 8)

        ESP_LOGI(FUNCTION_NAME, "Setting target position: %lx", position);
        ESP_LOGI(FUNCTION_NAME, "Speed: %d", speed);
        ESP_LOGI(FUNCTION_NAME, "Acceleration: %d", acceleration);
        ESP_LOGI(FUNCTION_NAME, "Mode: %s", absolute ? "Absolute" : "Relative");

        finalize_message(data);

        esp_err_t ret = enqueueMessage();
        return ret;
    }
};

#endif // SET_TARGET_POSITION_COMMAND_H