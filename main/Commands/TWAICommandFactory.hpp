#include "GenericCommandBuilder.hpp"
#include "RunMotorInSpeedModeCommandBuilder.hpp"
#include "SetTargetPositionCommandBuilder.hpp"
#include "utils.hpp"
#include <driver/twai.h>

class TWAICommandFactory
{
private:
    TWAICommandFactorySettings settings;

public:
    // Konstruktor setzt den Identifier
    TWAICommandFactory(TWAICommandFactorySettings settings) : settings(settings)
    {
    }

    void set_inQ(QueueHandle_t inQ)
    {
        settings.inQ = inQ;
    }

    SetHoldingCurrentCommandBuilder create_set_target_position_command()
    {
        return SetHoldingCurrentCommandBuilder(settings.id, settings.outQ, settings.inQ);
    }

    SetHomeParametersCommandBuilder create_run_motor_in_speed_mode_command()
    {
        return SetHomeParametersCommandBuilder(settings.id, settings.outQ, settings.inQ);
    }

    GenericCommandBuilder stop_motor_command()
    {
        ESP_LOGI(FUNCTION_NAME, "Stopping motor");
        return GenericCommandBuilder(settings.id, settings.outQ, settings.inQ, 0xF7);
    }

    GenericCommandBuilder create_query_motor_position_command()
    {
        ESP_LOGI(FUNCTION_NAME, "Querying motor position");
        return GenericCommandBuilder(settings.id, settings.outQ, settings.inQ, 0x30);
    }

    GenericCommandBuilder read_encoder_value()
    {
        ESP_LOGI(FUNCTION_NAME, "Reading encoder value");
        return GenericCommandBuilder(settings.id, settings.outQ, settings.inQ, 0x31);
    }

    GenericCommandBuilder read_realtime_speed_command()
    {
        ESP_LOGI(FUNCTION_NAME, "Reading Realtime Speed");
        return GenericCommandBuilder(settings.id, settings.outQ, settings.inQ, 0x32);
    }

    GenericCommandBuilder read_pulses_received_command()
    {
        ESP_LOGI(FUNCTION_NAME, "Reading Pulses Received");
        return GenericCommandBuilder(settings.id, settings.outQ, settings.inQ, 0x33);
    }

    GenericCommandBuilder read_shaft_angle_error_command()
    {
        ESP_LOGI(FUNCTION_NAME, "Reading Shaft Angle Error");
        return GenericCommandBuilder(settings.id, settings.outQ, settings.inQ, 0x39);
    }

    GenericCommandBuilder read_enable_pins_state_command()
    {
        ESP_LOGI(FUNCTION_NAME, "Reading Enable Pins State");
        return GenericCommandBuilder(settings.id, settings.outQ, settings.inQ, 0x3A);
    }

    GenericCommandBuilder release_shaft_protection_command()
    {
        ESP_LOGI(FUNCTION_NAME, "Releasing Shaft Protection");
        return GenericCommandBuilder(settings.id, settings.outQ, settings.inQ, 0x3D);
    }

    GenericCommandBuilder read_shaft_protection_state_command()
    {
        ESP_LOGI(FUNCTION_NAME, "Reading Shaft Protection State");
        return GenericCommandBuilder(settings.id, settings.outQ, settings.inQ, 0x3E);
    }

    GenericCommandBuilder calibrate_encoder_command()
    {
        ESP_LOGI(FUNCTION_NAME, "Calibrating encoder");
        return GenericCommandBuilder(settings.id, settings.outQ, settings.inQ, 0x80);
    }

    GenericCommandBuilder set_enable_pin_command(uint8_t enable)
    {
        ESP_LOGI(FUNCTION_NAME, "Setting enable pin %s", enable ? "on" : "off");
        return GenericCommandBuilder(settings.id, settings.outQ, settings.inQ, 0x85, {enable});
    }

    GenericCommandBuilder set_auto_screen_off_command(uint8_t enable)
    {
        ESP_LOGI(FUNCTION_NAME, "Setting auto screen %s", enable ? "on" : "off");
        return GenericCommandBuilder(settings.id, settings.outQ, settings.inQ, 0x87, {enable});
    }

    GenericCommandBuilder set_can_bit_rate_command(uint8_t bitRate)
    {
        ESP_LOGI(FUNCTION_NAME, "Setting CAN Bit Rate: %u", bitRate);
        return GenericCommandBuilder(settings.id, settings.outQ, settings.inQ, 0x8A, {bitRate});
    }

    GenericCommandBuilder set_holding_current_command(uint8_t holdCurrent)
    {
        ESP_LOGI(FUNCTION_NAME, "Setting Holding Current: %u", holdCurrent);
        return GenericCommandBuilder(settings.id, settings.outQ, settings.inQ, 0x9B, {holdCurrent});
    }

    GenericCommandBuilder set_locked_rotor_protection_command(uint8_t enable)
    {
        ESP_LOGI(FUNCTION_NAME, "Setting Locked Rotor Protection: %u", enable);
        return GenericCommandBuilder(settings.id, settings.outQ, settings.inQ, 0x88, {enable});
    }

    GenericCommandBuilder set_subdivision_command(uint8_t subdivision)
    {
        ESP_LOGI(FUNCTION_NAME, "Setting Subdivision: %u", subdivision);
        return GenericCommandBuilder(settings.id, settings.outQ, settings.inQ, 0x84, {subdivision});
    }

    GenericCommandBuilder set_subdivision_interpolation_command(uint8_t enable)
    {
        ESP_LOGI(FUNCTION_NAME, "Setting Subdivision Interpolation: %u", enable);
        return GenericCommandBuilder(settings.id, settings.outQ, settings.inQ, 0x89, {enable});
    }

    GenericCommandBuilder set_work_mode_command(uint8_t mode)
    {
        ESP_LOGI(FUNCTION_NAME, "Setting Work Mode: %u", mode);
        return GenericCommandBuilder(settings.id, settings.outQ, settings.inQ, 0x82, {mode});
    }

    GenericCommandBuilder set_rotation_direction_command(uint8_t direction)
    {
        ESP_LOGI(FUNCTION_NAME, "Setting Rotation Direction: %u", direction);
        return GenericCommandBuilder(settings.id, settings.outQ, settings.inQ, 0x86, {direction});
    }

    GenericCommandBuilder query_motor_status_command()
    {
        ESP_LOGI(FUNCTION_NAME, "Querying motor status");
        return GenericCommandBuilder(settings.id, settings.outQ, settings.inQ, 0xF1);
    }
};