#ifndef TWAI_COMMAND_FACTORY_H
#define TWAI_COMMAND_FACTORY_H

#include "GenericCommandBuilder.hpp"
#include "RunMotorInSpeedModeCommandBuilder.hpp"
#include "SetTargetPositionCommandBuilder.hpp"
#include "TypeDefs.hpp"
#include "utils.hpp"
#include <driver/twai.h>

class TWAICommandFactory
{
private:
    TWAICommandFactorySettings settings;

    SetTargetPositionCommandBuilder *setTargetPositionCommandBuilder;
    RunMotorInSpeedModeCommandBuilder *runMotorInSpeedModeCommandBuilder;

public:
    // Konstruktor setzt den Identifier
    TWAICommandFactory(TWAICommandFactorySettings &settings) : settings(settings)
    {
        ESP_LOGI(FUNCTION_NAME, "TWAICommandFactory constructor called");
    }

    SetTargetPositionCommandBuilder create_set_target_position_command()
    {
        ESP_LOGI(FUNCTION_NAME, "Creating Set Target Position Command");
        setTargetPositionCommandBuilder = new SetTargetPositionCommandBuilder(settings);
        return setTargetPositionCommandBuilder->init_new_command();
    }

    RunMotorInSpeedModeCommandBuilder create_run_motor_in_speed_mode_command()
    {
        ESP_LOGI(FUNCTION_NAME, "Creating Run Motor In Speed Mode Command");
        runMotorInSpeedModeCommandBuilder = new RunMotorInSpeedModeCommandBuilder(settings);
        return runMotorInSpeedModeCommandBuilder->init_new_command();
    }

    GenericCommandBuilder *generate_new_generic_builder()
    {
        ESP_LOGI(FUNCTION_NAME, "Creating Generic Command Builder");
        GenericCommandBuilder *genericCommandBuilder = new GenericCommandBuilder(settings);
        return genericCommandBuilder;
    }

    GenericCommandBuilder stop_motor_command()
    {
        ESP_LOGI(FUNCTION_NAME, "Stopping motor");
        return generate_new_generic_builder()->init_new_command(0xF7);
    }

    GenericCommandBuilder create_query_motor_position_command()
    {
        ESP_LOGI(FUNCTION_NAME, "Querying motor position");
        return generate_new_generic_builder()->init_new_command(0x30);
    }

    GenericCommandBuilder read_encoder_value()
    {
        ESP_LOGI(FUNCTION_NAME, "Reading encoder value");
        return generate_new_generic_builder()->init_new_command(0x31);
    }

    GenericCommandBuilder read_realtime_speed_command()
    {
        ESP_LOGI(FUNCTION_NAME, "Reading Realtime Speed");
        return generate_new_generic_builder()->init_new_command(0x32);
    }

    GenericCommandBuilder read_pulses_received_command()
    {
        ESP_LOGI(FUNCTION_NAME, "Reading Pulses Received");
        return generate_new_generic_builder()->init_new_command(0x33);
    }

    GenericCommandBuilder read_shaft_angle_error_command()
    {
        ESP_LOGI(FUNCTION_NAME, "Reading Shaft Angle Error");
        return generate_new_generic_builder()->init_new_command(0x39);
    }

    GenericCommandBuilder read_enable_pins_state_command()
    {
        ESP_LOGI(FUNCTION_NAME, "Reading Enable Pins State");
        return generate_new_generic_builder()->init_new_command(0x3A);
    }

    GenericCommandBuilder release_shaft_protection_command()
    {
        ESP_LOGI(FUNCTION_NAME, "Releasing Shaft Protection");
        return generate_new_generic_builder()->init_new_command(0x3D);
    }

    GenericCommandBuilder read_shaft_protection_state_command()
    {
        ESP_LOGI(FUNCTION_NAME, "Reading Shaft Protection State");
        return generate_new_generic_builder()->init_new_command(0x3E);
    }

    GenericCommandBuilder calibrate_encoder_command()
    {
        ESP_LOGI(FUNCTION_NAME, "Calibrating encoder");
        return generate_new_generic_builder()->init_new_command(0x80);
    }

    GenericCommandBuilder set_enable_pin_command(uint8_t enable)
    {
        ESP_LOGI(FUNCTION_NAME, "Setting enable pin %s", enable ? "on" : "off");
        return generate_new_generic_builder()->init_new_command(0x85, {enable});
    }

    GenericCommandBuilder set_auto_screen_off_command(uint8_t enable)
    {
        ESP_LOGI(FUNCTION_NAME, "Setting auto screen %s", enable ? "on" : "off");
        return generate_new_generic_builder()->init_new_command(0x87, {enable});
    }

    GenericCommandBuilder set_can_bit_rate_command(uint8_t bitRate)
    {
        ESP_LOGI(FUNCTION_NAME, "Setting CAN Bit Rate: %u", bitRate);
        return generate_new_generic_builder()->init_new_command(0x8A, {bitRate});
    }

    GenericCommandBuilder set_holding_current_command(uint8_t holdCurrent)
    {
        ESP_LOGI(FUNCTION_NAME, "Setting Holding Current: %u", holdCurrent);
        return generate_new_generic_builder()->init_new_command(0x9B, {holdCurrent});
    }

    GenericCommandBuilder set_locked_rotor_protection_command(uint8_t enable)
    {
        ESP_LOGI(FUNCTION_NAME, "Setting Locked Rotor Protection: %u", enable);
        return generate_new_generic_builder()->init_new_command(0x88, {enable});
    }

    GenericCommandBuilder set_subdivision_command(uint8_t subdivision)
    {
        ESP_LOGI(FUNCTION_NAME, "Setting Subdivision: %u", subdivision);
        return generate_new_generic_builder()->init_new_command(0x84, {subdivision});
    }

    GenericCommandBuilder set_subdivision_interpolation_command(uint8_t enable)
    {
        ESP_LOGI(FUNCTION_NAME, "Setting Subdivision Interpolation: %u", enable);
        return generate_new_generic_builder()->init_new_command(0x89, {enable});
    }

    GenericCommandBuilder set_work_mode_command(uint8_t mode)
    {
        ESP_LOGI(FUNCTION_NAME, "Setting Work Mode: %u", mode);
        return generate_new_generic_builder()->init_new_command(0x82, {mode});
    }

    GenericCommandBuilder set_rotation_direction_command(uint8_t direction)
    {
        ESP_LOGI(FUNCTION_NAME, "Setting Rotation Direction: %u", direction);
        return generate_new_generic_builder()->init_new_command(0x86, {direction});
    }

    GenericCommandBuilder query_motor_status_command()
    {
        ESP_LOGI(FUNCTION_NAME, "Querying motor status");
        return generate_new_generic_builder()->init_new_command(0xF1);
    }
};

#endif // TWAI_COMMAND_FACTORY_H