#ifndef SET_TARGET_POSITION_COMMAND_BUILDER_H
#define SET_TARGET_POSITION_COMMAND_BUILDER_H

#include "../TypeDefs.hpp"
#include "TWAICommandBuilderBase.hpp"
#include "esp_log.h"
#include "freertos/queue.h"

class SetTargetPositionCommandBuilder : public TWAICommandBuilderBase<SetTargetPositionCommandBuilder>
{
private:
    uint32_t position;
    uint8_t speed;
    uint8_t acceleration;
    bool absolute;

public:
    SetTargetPositionCommandBuilder(std::shared_ptr<TWAICommandFactorySettings> settings) : TWAICommandBuilderBase<SetTargetPositionCommandBuilder>(settings, 0x00)
    {
        set_data_length_code(8);
        create_msg_data();
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
        command_code = absolute ? 0xF5 : 0xF4;
        return *this;
    }

    esp_err_t build_twai_message()
    {
        data[0] = command_code;            // Befehlscode für Position mode4: absolute motion by axis
        data[1] = (speed >> 8) & 0x7F;     // Combine direction bit with the upper 7 bits of speed
        data[2] = speed & 0xFF;            // Lower 8 bits of speed
        data[3] = acceleration;            // Beschleunigung
        data[4] = (position >> 16) & 0xFF; // Obere 8 Bits der Position
        data[5] = (position >> 8) & 0xFF;  // Mittlere 8 Bits der Position
        data[6] = position & 0xFF;         // Untere 8 Bits der Position
                                           // data[7] = calculate_crc(data, 8)

        ESP_LOGI(FUNCTION_NAME, "Setting target position: %lx", position);
        ESP_LOGI(FUNCTION_NAME, "Speed: %d", speed);
        ESP_LOGI(FUNCTION_NAME, "Acceleration: %d", acceleration);
        ESP_LOGI(FUNCTION_NAME, "Mode: %s", absolute ? "Absolute" : "Relative");

        return ESP_OK;
    }
};

#endif // SET_TARGET_POSITION_COMMAND_H