#ifndef TWAI_COMMAND_FACTORY_H
#define TWAI_COMMAND_FACTORY_H

#include "../common/utils.hpp"
#include "GenericCommand.hpp"
#include "RunMotorInSpeedModeCommand.hpp"
#include "SetTargetPositionCommand.hpp"
#include "TypeDefs.hpp"
#include <driver/twai.h>
#include <memory>
#include <typeinfo>

class CommandFactory
{
private:
    std::shared_ptr<CommandFactorySettings> settings;
    SetTargetPositionCommand *setTargetPositionCommand;
    RunMotorInSpeedModeCommand *runMotorInSpeedModeCommand;
    SemaphoreHandle_t factory_mutex;

public:
    // Konstruktor setzt den Identifier
    CommandFactory(std::shared_ptr<CommandFactorySettings> settings) : settings(settings)
    {
        factory_mutex = xSemaphoreCreateMutex();

        ESP_LOGI(FUNCTION_NAME, "CommandFactory constructor called");
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

    ~CommandFactory()
    {
        ESP_LOGW(FUNCTION_NAME, "CommandFactory destructor called");
    }

    esp_err_t check_settings()
    {
        esp_err_t ret = ESP_OK;
        // Assertions to catch programming errors
        assert(settings && "CommandFactorySettings is null");

        return ret;
    }

    SetTargetPositionCommand *create_set_target_position_command()
    {
        ESP_LOGI(FUNCTION_NAME, "Creating Set Target Position Command");

        return new SetTargetPositionCommand(settings);
    }

    RunMotorInSpeedModeCommand *run_motor_in_speed_mode_command()
    {
        ESP_LOGI(FUNCTION_NAME, "Creating Run Motor In Speed Mode Command");
        return new RunMotorInSpeedModeCommand(settings);
    }

    GenericCommand *generate_new_generic_command(CommandIds command_code)
    {
        ESP_LOGI(FUNCTION_NAME, "Creating Generic Command");
        if (xSemaphoreTake(factory_mutex, portMAX_DELAY) == pdTRUE)
        {
            // Check if settings is null
            if (!settings)
            {
                ESP_LOGE(FUNCTION_NAME, "Settings pointer is null");
                return nullptr; // Return early to avoid dereferencing a null pointer
            }

            // Log the integer value of the command_code safely
            uint8_t command_code_int = static_cast<uint8_t>(command_code);
            ESP_LOGI(FUNCTION_NAME, "Creating Generic Command for command code: 0x%02X", command_code_int);
            // ESP_LOGI(FUNCTION_NAME, "Command: 0x%02X", cmd_code);

            // ESP_LOGI(FUNCTION_NAME, "Command: %s", GET_CMD(command_code_int));
            // ESP_LOGI(FUNCTION_NAME, "Command: %s", GET_CMD(command_code));
            // ESP_LOGI(FUNCTION_NAME, "Command: %s", GET_CMD(cmd_code));

            // ESP_LOGI(FUNCTION_NAME, "Command: %s", GET_CMDPTR(&cmd_code));
            // ESP_LOGI(FUNCTION_NAME, "Command: %s", magic_enum::enum_name(command_code).data());
            // ESP_LOGI(FUNCTION_NAME, "Command: %s", magic_enum::enum_name(cmd_code).data());

            // ESP_LOGI(FUNCTION_NAME, "Creating Generic Command for command code: %d, \t command name: %s", command_code_int, GET_CMD(command_code));
            GenericCommand *command = new GenericCommand(settings, command_code);
            // Check if the command was created successfully
            if (!command)
            {
                ESP_LOGE(FUNCTION_NAME, "Failed to create Generic Command");
                return nullptr; // Return early if the command creation failed
            }
            xSemaphoreGive(factory_mutex);
            return command;
        }
        else
        {
            ESP_LOGE(FUNCTION_NAME, "Failed to take mutex");
            return nullptr;
        }
    }

    /*
        GenericCommand *generate_new_generic_command(CommandIds command_code)
        {
            ESP_LOGI(FUNCTION_NAME, "Creating Generic Command");
            return new GenericCommand(settings, command_code);
        }

        GenericCommand *generate_new_generic_command(CommandIds command_code, std::vector<uint8_t> payload)
        {
            ESP_LOGI(FUNCTION_NAME, "Creating Generic Command with payload");
            return new GenericCommand(settings, command_code, payload);
        }

        GenericCommand *stop_motor_command()
        {
            ESP_LOGI(FUNCTION_NAME, "Stopping motor");
            return new GenericCommand(settings, EMERGENCY_STOP);
        }

        CommandBase<GenericCommand> *query_motor_position_command()
        {
            ESP_LOGI(FUNCTION_NAME, "Querying motor position");
            return new GenericCommand(settings, 0x30);
        }

        CommandBase<GenericCommand> *read_encoder_value()
        {
            ESP_LOGI(FUNCTION_NAME, "Reading encoder value");
            return new GenericCommand(settings, 0x31);
        }

        CommandBase<GenericCommand> *read_realtime_speed_command()
        {
            ESP_LOGI(FUNCTION_NAME, "Reading Realtime Speed");
            return new GenericCommand(settings, 0x32);
        }

        CommandBase<GenericCommand> *read_pulses_received_command()
        {
            ESP_LOGI(FUNCTION_NAME, "Reading Pulses Received");
            return new GenericCommand(settings, 0x33);
        }

        CommandBase<GenericCommand> *read_shaft_angle_error_command()
        {
            ESP_LOGI(FUNCTION_NAME, "Reading Shaft Angle Error");
            return new GenericCommand(settings, 0x39);
        }

        CommandBase<GenericCommand> *read_enable_pins_state_command()
        {
            ESP_LOGI(FUNCTION_NAME, "Reading Enable Pins State");
            return new GenericCommand(settings, 0x3A);
        }

        CommandBase<GenericCommand> *release_shaft_protection_command()
        {
            ESP_LOGI(FUNCTION_NAME, "Releasing Shaft Protection");
            return new GenericCommand(settings, 0x3D);
        }

        CommandBase<GenericCommand> *read_shaft_protection_state_command()
        {
            ESP_LOGI(FUNCTION_NAME, "Reading Shaft Protection State");
            return new GenericCommand(settings, 0x3E);
        }

        CommandBase<GenericCommand> *calibrate_encoder_command()
        {
            ESP_LOGI(FUNCTION_NAME, "Calibrating encoder");
            return new GenericCommand(settings, 0x80);
        }

        CommandBase<GenericCommand> *query_motor_status_command()
        {
            ESP_LOGI(FUNCTION_NAME, "Querying motor status");
            return new GenericCommand(settings, 0xF1);
        }

        CommandBase<GenericCommand> *set_work_mode_command(uint8_t mode)
        {
            ESP_LOGI(FUNCTION_NAME, "Setting Work Mode: %u", mode);
            return new GenericCommand(settings, 0x82, std::vector<uint8_t>{mode});
        }

        CommandBase<GenericCommand> *set_working_current(uint16_t current_ma)
        {
            ESP_LOGI(FUNCTION_NAME, "Setting Working Current: %u", current_ma);
            return new GenericCommand(settings, 0x83, std::vector<uint16_t>{current_ma});
        }

        CommandBase<GenericCommand> *set_subdivision_command(uint8_t subdivision)
        {
            ESP_LOGI(FUNCTION_NAME, "Setting Subdivision: %u", subdivision);
            return new GenericCommand(settings, 0x84, std::vector<uint8_t>{subdivision});
        }

        CommandBase<GenericCommand> *set_enable_pin_command(uint8_t enable)
        {
            ESP_LOGI(FUNCTION_NAME, "Setting enable pin %s", enable ? "on" : "off");
            return new GenericCommand(settings, 0x85, std::vector<uint8_t>{enable});
        }

        CommandBase<GenericCommand> *set_rotation_direction_command(uint8_t direction)
        {
            ESP_LOGI(FUNCTION_NAME, "Setting Rotation Direction: %u", direction);
            return new GenericCommand(settings, 0x86, std::vector<uint8_t>{direction});
        }

        CommandBase<GenericCommand> *set_auto_screen_off_command(uint8_t enable)
        {
            ESP_LOGI(FUNCTION_NAME, "Setting auto screen %s", enable ? "on" : "off");
            return new GenericCommand(settings, 0x87, std::vector<uint8_t>{enable});
        }

        CommandBase<GenericCommand> *set_locked_rotor_protection_command(uint8_t enable)
        {
            ESP_LOGI(FUNCTION_NAME, "Setting Locked Rotor Protection: %u", enable);
            return new GenericCommand(settings, 0x88, std::vector<uint8_t>{enable});
        }

        CommandBase<GenericCommand> *set_subdivision_interpolation_command(uint8_t enable)
        {
            ESP_LOGI(FUNCTION_NAME, "Setting Subdivision Interpolation: %u", enable);
            return new GenericCommand(settings, 0x89, std::vector<uint8_t>{enable});
        }

        CommandBase<GenericCommand> *set_can_bit_rate_command(uint8_t bitRate)
        {
            ESP_LOGI(FUNCTION_NAME, "Setting CAN Bit Rate: %u", bitRate);
            return new GenericCommand(settings, 0x8A, std::vector<uint8_t>{bitRate});
        }

        CommandBase<GenericCommand> *set_holding_current_command(uint8_t holdCurrent)
        {
            ESP_LOGI(FUNCTION_NAME, "Setting Holding Current: %u", holdCurrent);
            return new GenericCommand(settings, 0x9B, std::vector<uint8_t>{holdCurrent});
        }
    */
};

#endif // TWAI_COMMAND_FACTORY_H