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

public:
    // Konstruktor setzt den Identifier
    CommandFactory(std::shared_ptr<CommandFactorySettings> settings) : settings(settings)
    {
        ESP_LOGI(FUNCTION_NAME, "CommandFactory constructor called");
    }

    ~CommandFactory()
    {
        ESP_LOGW(FUNCTION_NAME, "CommandFactory destructor called");
    }

    GenericCommand *create_command(CommandIds command_code)
    {
        ESP_LOGI(FUNCTION_NAME, "Creating Generic Command for motor: %lu \t command id: %s", settings->id, magic_enum::enum_name(command_code).data());

        RETURN_NPTR_IF(settings == nullptr);

        uint8_t command_code_int = static_cast<uint8_t>(command_code);
        ESP_LOGI(FUNCTION_NAME, "Creating Generic Command for command code: 0x%02X", command_code_int);
        GenericCommand *command = new GenericCommand(settings, command_code);
        RETURN_NPTR_IF(command == nullptr);

        return command;
    }
};

#endif // TWAI_COMMAND_FACTORY_H