#pragma once

#include "Events.hpp"
#include "SetRunModeCommand.h"
#include "TypeDefs.hpp"
#include "WsCommandDefs.hpp"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "utils.hpp"

class WsCommandFactory
{
public:
    explicit WsCommandFactory(ws_command_config_map_t config_map) : config_map(config_map)
    {
        factory_mutex = xSemaphoreCreateMutex();
    };

    ~WsCommandFactory() {};

    esp_err_t create(ws_command_id cmd, ws_payload_t payload)
    {
        this->cmd = cmd;
        auto it = config_map.find(cmd);
        CommandEventConfig cmd_config = it->second;

        if (cmd == ws_command_id::SET_RUNMODE)
        {
            ESP_LOGD(FUNCTION_NAME, "Creating SET_RUN_MODE command");
            SetRunmodeCommand *command = new SetRunmodeCommand();
            ERRCHECK_OR_DELETE_COMMAND(command->set_config(cmd_config));
            ERRCHECK_OR_DELETE_COMMAND(command->set_run_mode(std::get<RunMode>(payload)));
            command->post(cmd_config.loop, cmd_config.event_base, cmd_config.event_id);
        }
        return ESP_OK;
    }

private:
    ws_command_id cmd;
    ws_command_config_map_t config_map;
    SemaphoreHandle_t factory_mutex;
};