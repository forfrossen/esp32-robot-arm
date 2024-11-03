#pragma once

#include "Events.hpp"
#include "SetRunLevelCommand.h"
#include "TypeDefs.hpp"
#include "WsCommandDefs.hpp"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "utils.hpp"

class WsCommandFactory
{
public:
    explicit WsCommandFactory(
        ws_command_config_map_t config_map,
        esp_event_loop_handle_t response_loop,
        esp_event_base_t response_event_base,
        int32_t response_event_id)
        : config_map(config_map),
          response_loop(response_loop),
          response_event_base(response_event_base),
          response_event_id(response_event_id)
    {
        factory_mutex = xSemaphoreCreateMutex();
    };

    ~WsCommandFactory() {};

    esp_err_t create(ws_command_id cmd, nlohmann::json payload, int id, std::string client_id)
    {
        this->cmd = cmd;
        auto it = config_map.find(cmd);
        rpc_event_config_t cmd_config = it->second;
        rpc_event_config_t cmd_response_config = {response_loop, response_event_base, response_event_id};

        if (cmd == ws_command_id::SET_RUNMODE)
        {
            ESP_LOGD(FUNCTION_NAME, "Creating SET_RUN_MODE command");
            SetRunLevelCommand *command = new SetRunLevelCommand(id, client_id);
            ERRCHECK_OR_DELETE_COMMAND(command->set_config(cmd_config));
            ERRCHECK_OR_DELETE_COMMAND(command->set_response_config(cmd_response_config));
            ERRCHECK_OR_DELETE_COMMAND(command->set_run_level(payload));

            command->post();
        }

        return ESP_OK;
    }

private:
    ws_command_id cmd;
    ws_command_config_map_t config_map;
    SemaphoreHandle_t factory_mutex;

    esp_event_loop_handle_t response_loop;
    esp_event_base_t response_event_base;
    int32_t response_event_id;
};