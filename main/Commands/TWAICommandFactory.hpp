#include "GenericCommandBuilder.hpp"
#include "RunMotorInSpeedModeCommandBuilder.hpp"
#include "SetTargetPositionCommandBuilder.hpp"
#include "freertos/queue.h" // Include the header file that defines QueueHandle_t
#include <driver/twai.h>

class TWAICommandFactory
{
private:
    uint32_t id;
    QueueHandle_t outQ;

public:
    // Konstruktor setzt den Identifier
    TWAICommandFactory(uint32_t id, QueueHandle_t outQ) : id(id), outQ(outQ)
    {
    }

    SetTargetPositionCommandBuilder create_set_target_position_command()
    {
        return SetTargetPositionCommandBuilder(id, outQ);
    }

    RunMotorInSpeedModeCommandBuilder create_run_motor_in_speed_mode_command()
    {
        return RunMotorInSpeedModeCommandBuilder(id, outQ);
    }

    GenericCommandBuilder stop_motor_command()
    {
        ESP_LOGI(FUNCTION_NAME, "Stopping motor");
        return GenericCommandBuilder(id, outQ, 0xF7);
    }

    GenericCommandBuilder create_query_motor_position_command()
    {
        ESP_LOGI(FUNCTION_NAME, "Querying motor position");
        return GenericCommandBuilder(id, outQ, 0x30);
    }

    GenericCommandBuilder query_motor_status_command()
    {
        ESP_LOGI(FUNCTION_NAME, "Querying motor status");
        return GenericCommandBuilder(id, outQ, 0xF1);
    }

    GenericCommandBuilder set_auto_screen_off_command(uint8_t enable)
    {
        ESP_LOGI(FUNCTION_NAME, "Setting auto screen %s", enable ? "on" : "off");
        return GenericCommandBuilder(id, outQ, 0x87, {enable});
    }

    GenericCommandBuilder set_enable_pin_command(uint8_t enable)
    {
        ESP_LOGI(FUNCTION_NAME, "Setting enable pin %s", enable ? "on" : "off");
        return GenericCommandBuilder(id, outQ, 0x85, {enable});
    }

    GenericCommandBuilder set_can_bit_rate_command(uint8_t bitRate)
    {
        ESP_LOGI(FUNCTION_NAME, "Setting CAN Bit Rate: %u", bitRate);
        return GenericCommandBuilder(id, outQ, 0x8A, {bitRate});
    }

    GenericCommandBuilder set_holding_current_command(uint8_t holdCurrent)
    {
        ESP_LOGI(FUNCTION_NAME, "Setting Holding Current: %u", holdCurrent);
        return GenericCommandBuilder(id, outQ, 0x9B, {holdCurrent});
    }

    GenericCommandBuilder set_locked_rotor_protection_command(uint8_t enable)
    {
        ESP_LOGI(FUNCTION_NAME, "Setting Locked Rotor Protection: %u", enable);
        return GenericCommandBuilder(id, outQ, 0x88, {enable});
    }

    GenericCommandBuilder set_subdivision_command(uint8_t subdivision)
    {
        ESP_LOGI(FUNCTION_NAME, "Setting Subdivision: %u", subdivision);
        return GenericCommandBuilder(id, outQ, 0x84, {subdivision});
    }

    GenericCommandBuilder set_subdivision_interpolation_command(uint8_t enable)
    {
        ESP_LOGI(FUNCTION_NAME, "Setting Subdivision Interpolation: %u", enable);
        return GenericCommandBuilder(id, outQ, 0x89, {enable});
    }

    GenericCommandBuilder set_work_mode_command(uint8_t mode)
    {
        ESP_LOGI(FUNCTION_NAME, "Setting Work Mode: %u", mode);
        return GenericCommandBuilder(id, outQ, 0x82, {mode});
    }

    GenericCommandBuilder set_rotation_direction_command(uint8_t direction)
    {
        ESP_LOGI(FUNCTION_NAME, "Setting Rotation Direction: %u", direction);
        return GenericCommandBuilder(id, outQ, 0x86, {direction});
    }
};