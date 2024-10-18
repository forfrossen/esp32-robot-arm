#ifndef TWAI_COMMAND_FACTORY_H
#define TWAI_COMMAND_FACTORY_H

#include "../common/utils.hpp"
#include "GenericCommand.hpp"
#include "RunMotorInSpeedModeCommand.hpp"
#include "SetTargetPositionCommand.hpp"
#include "TypeDefs.hpp"
#include <driver/twai.h>
#include <iostream>
#include <memory>
#include <type_traits>
#include <typeinfo>

class CommandFactory
{
private:
    std::shared_ptr<CommandFactorySettings> settings;
    std::shared_ptr<SetTargetPositionCommand> set_target_position_command;

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

    template <typename... Args>
    GenericCommand *create_command(CommandIds command_id, Args &&...args)
    {
        ESP_LOGI(FUNCTION_NAME, "Creating Generic Command for motor: %lu \t command id: %s", settings->id, magic_enum::enum_name(command_id).data());

        RETURN_NPTR_IF(settings == nullptr);

        uint8_t command_id_int = static_cast<uint8_t>(command_id);
        ESP_LOGI(FUNCTION_NAME, "Creating Generic Command for command code: 0x%02X", command_id_int);

        GenericCommand *command = new GenericCommand(settings, command_id, std::forward<Args>(args)...);
        RETURN_NPTR_IF(command->is_error);

        return command;
    }

    std::shared_ptr<SetTargetPositionCommand> create_set_target_position_command(bool absolute)
    {
        ESP_LOGI(FUNCTION_NAME, "Creating Set Target Position Command");
        auto cmd = std::make_shared<SetTargetPositionCommand>(settings);
        cmd->set_absolute(absolute);
        return cmd;
    }
};

#endif // TWAI_COMMAND_FACTORY_H