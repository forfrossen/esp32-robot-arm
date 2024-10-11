#ifndef TWAI_COMMAND_FACTORY_H
#define TWAI_COMMAND_FACTORY_H

#include "../common/utils.hpp"
#include "GenericCommandBuilder.hpp"
#include "RunMotorInSpeedModeCommandBuilder.hpp"
#include "SetTargetPositionCommandBuilder.hpp"
#include "TypeDefs.hpp"
#include <driver/twai.h>
#include <memory>
#include <typeinfo>

class TWAICommandFactory
{
private:
    std::shared_ptr<TWAICommandFactorySettings> settings;
    SetTargetPositionCommandBuilder *setTargetPositionCommandBuilder;
    RunMotorInSpeedModeCommandBuilder *runMotorInSpeedModeCommandBuilder;

public:
    // Konstruktor setzt den Identifier
    TWAICommandFactory(std::shared_ptr<TWAICommandFactorySettings> settings) : settings(settings)
    {
        ESP_LOGI(FUNCTION_NAME, "TWAICommandFactory constructor called");
        esp_err_t ret = check_settings();
        if (ret != ESP_OK)
        {
            ESP_LOGE(FUNCTION_NAME, "Settings not OK!");
        }
        else
        {
            ESP_LOGI(FUNCTION_NAME, "Settings OK!");
        }
    }

    ~TWAICommandFactory()
    {
        ESP_LOGW(FUNCTION_NAME, "TWAICommandFactory destructor called");
    }

    esp_err_t check_settings()
    {
        esp_err_t ret = ESP_OK;
        // Assertions to catch programming errors
        assert(settings && "TWAICommandFactorySettings is null");

        return ret;
    }

    SetTargetPositionCommandBuilder *create_set_target_position_command()
    {
        ESP_LOGI(FUNCTION_NAME, "Creating Set Target Position Command");

        return new SetTargetPositionCommandBuilder(settings);
    }

    RunMotorInSpeedModeCommandBuilder *run_motor_in_speed_mode_command()
    {
        ESP_LOGI(FUNCTION_NAME, "Creating Run Motor In Speed Mode Command");
        return new RunMotorInSpeedModeCommandBuilder(settings);
    }

    GenericCommandBuilder *generate_new_generic_builder(CommandIds command_code)
    {
        ESP_LOGI(FUNCTION_NAME, "Creating Generic Command Builder");
        return new GenericCommandBuilder(settings, command_code);
    }

    /*
        GenericCommandBuilder *generate_new_generic_builder(CommandIds command_code)
        {
            ESP_LOGI(FUNCTION_NAME, "Creating Generic Command Builder");
            return new GenericCommandBuilder(settings, command_code);
        }

        GenericCommandBuilder *generate_new_generic_builder(CommandIds command_code, std::vector<uint8_t> payload)
        {
            ESP_LOGI(FUNCTION_NAME, "Creating Generic Command Builder with payload");
            return new GenericCommandBuilder(settings, command_code, payload);
        }

        GenericCommandBuilder *stop_motor_command()
        {
            ESP_LOGI(FUNCTION_NAME, "Stopping motor");
            return new GenericCommandBuilder(settings, CommandIds::EMERGENCY_STOP);
        }

        TWAICommandBuilderBase<GenericCommandBuilder> *query_motor_position_command()
        {
            ESP_LOGI(FUNCTION_NAME, "Querying motor position");
            return new GenericCommandBuilder(settings, 0x30);
        }

        TWAICommandBuilderBase<GenericCommandBuilder> *read_encoder_value()
        {
            ESP_LOGI(FUNCTION_NAME, "Reading encoder value");
            return new GenericCommandBuilder(settings, 0x31);
        }

        TWAICommandBuilderBase<GenericCommandBuilder> *read_realtime_speed_command()
        {
            ESP_LOGI(FUNCTION_NAME, "Reading Realtime Speed");
            return new GenericCommandBuilder(settings, 0x32);
        }

        TWAICommandBuilderBase<GenericCommandBuilder> *read_pulses_received_command()
        {
            ESP_LOGI(FUNCTION_NAME, "Reading Pulses Received");
            return new GenericCommandBuilder(settings, 0x33);
        }

        TWAICommandBuilderBase<GenericCommandBuilder> *read_shaft_angle_error_command()
        {
            ESP_LOGI(FUNCTION_NAME, "Reading Shaft Angle Error");
            return new GenericCommandBuilder(settings, 0x39);
        }

        TWAICommandBuilderBase<GenericCommandBuilder> *read_enable_pins_state_command()
        {
            ESP_LOGI(FUNCTION_NAME, "Reading Enable Pins State");
            return new GenericCommandBuilder(settings, 0x3A);
        }

        TWAICommandBuilderBase<GenericCommandBuilder> *release_shaft_protection_command()
        {
            ESP_LOGI(FUNCTION_NAME, "Releasing Shaft Protection");
            return new GenericCommandBuilder(settings, 0x3D);
        }

        TWAICommandBuilderBase<GenericCommandBuilder> *read_shaft_protection_state_command()
        {
            ESP_LOGI(FUNCTION_NAME, "Reading Shaft Protection State");
            return new GenericCommandBuilder(settings, 0x3E);
        }

        TWAICommandBuilderBase<GenericCommandBuilder> *calibrate_encoder_command()
        {
            ESP_LOGI(FUNCTION_NAME, "Calibrating encoder");
            return new GenericCommandBuilder(settings, 0x80);
        }

        TWAICommandBuilderBase<GenericCommandBuilder> *query_motor_status_command()
        {
            ESP_LOGI(FUNCTION_NAME, "Querying motor status");
            return new GenericCommandBuilder(settings, 0xF1);
        }

        TWAICommandBuilderBase<GenericCommandBuilder> *set_work_mode_command(uint8_t mode)
        {
            ESP_LOGI(FUNCTION_NAME, "Setting Work Mode: %u", mode);
            return new GenericCommandBuilder(settings, 0x82, std::vector<uint8_t>{mode});
        }

        TWAICommandBuilderBase<GenericCommandBuilder> *set_working_current(uint16_t current_ma)
        {
            ESP_LOGI(FUNCTION_NAME, "Setting Working Current: %u", current_ma);
            return new GenericCommandBuilder(settings, 0x83, std::vector<uint16_t>{current_ma});
        }

        TWAICommandBuilderBase<GenericCommandBuilder> *set_subdivision_command(uint8_t subdivision)
        {
            ESP_LOGI(FUNCTION_NAME, "Setting Subdivision: %u", subdivision);
            return new GenericCommandBuilder(settings, 0x84, std::vector<uint8_t>{subdivision});
        }

        TWAICommandBuilderBase<GenericCommandBuilder> *set_enable_pin_command(uint8_t enable)
        {
            ESP_LOGI(FUNCTION_NAME, "Setting enable pin %s", enable ? "on" : "off");
            return new GenericCommandBuilder(settings, 0x85, std::vector<uint8_t>{enable});
        }

        TWAICommandBuilderBase<GenericCommandBuilder> *set_rotation_direction_command(uint8_t direction)
        {
            ESP_LOGI(FUNCTION_NAME, "Setting Rotation Direction: %u", direction);
            return new GenericCommandBuilder(settings, 0x86, std::vector<uint8_t>{direction});
        }

        TWAICommandBuilderBase<GenericCommandBuilder> *set_auto_screen_off_command(uint8_t enable)
        {
            ESP_LOGI(FUNCTION_NAME, "Setting auto screen %s", enable ? "on" : "off");
            return new GenericCommandBuilder(settings, 0x87, std::vector<uint8_t>{enable});
        }

        TWAICommandBuilderBase<GenericCommandBuilder> *set_locked_rotor_protection_command(uint8_t enable)
        {
            ESP_LOGI(FUNCTION_NAME, "Setting Locked Rotor Protection: %u", enable);
            return new GenericCommandBuilder(settings, 0x88, std::vector<uint8_t>{enable});
        }

        TWAICommandBuilderBase<GenericCommandBuilder> *set_subdivision_interpolation_command(uint8_t enable)
        {
            ESP_LOGI(FUNCTION_NAME, "Setting Subdivision Interpolation: %u", enable);
            return new GenericCommandBuilder(settings, 0x89, std::vector<uint8_t>{enable});
        }

        TWAICommandBuilderBase<GenericCommandBuilder> *set_can_bit_rate_command(uint8_t bitRate)
        {
            ESP_LOGI(FUNCTION_NAME, "Setting CAN Bit Rate: %u", bitRate);
            return new GenericCommandBuilder(settings, 0x8A, std::vector<uint8_t>{bitRate});
        }

        TWAICommandBuilderBase<GenericCommandBuilder> *set_holding_current_command(uint8_t holdCurrent)
        {
            ESP_LOGI(FUNCTION_NAME, "Setting Holding Current: %u", holdCurrent);
            return new GenericCommandBuilder(settings, 0x9B, std::vector<uint8_t>{holdCurrent});
        }
    */
};

#endif // TWAI_COMMAND_FACTORY_H