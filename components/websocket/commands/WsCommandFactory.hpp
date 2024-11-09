#pragma once

#include "Events.hpp"
#include "MotorControlCommand.h"
#include "RpcCommand.h"
#include "SetRunLevelCommand.h"
#include "TypeDefs.hpp"
#include "WsCommandDefs.hpp"
#include "utils.hpp"
#include <esp_check.h>
#include <esp_err.h>
#include <esp_event.h>
#include <esp_log.h>
#include <magic_enum.hpp>
#include <nlohmann/json.hpp>
#include <variant>

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
    }

    ~WsCommandFactory() {}

    esp_err_t create(httpd_req_t *req, ws_message_t msg)
    {
        if (std::holds_alternative<system_command_id_t>(msg.command))
        {
            ESP_LOGD(TAG, "Command: %s", magic_enum::enum_name(std::get<system_command_id_t>(msg.command)).data());
            return create_command_and_send<system_command_id_t>(req, msg);
        }
        if (std::holds_alternative<motor_command_id_t>(msg.command))
        {
            ESP_LOGD(TAG, "Command: %s", magic_enum::enum_name(std::get<motor_command_id_t>(msg.command)).data());
            return create_command_and_send<motor_command_id_t>(req, msg);
        }
        return ESP_ERR_INVALID_ARG;
    }

private:
    template <typename CommandType>
    esp_err_t create_command_and_send(httpd_req_t *req, ws_message_t msg)
    {
        ESP_LOGD(TAG, "Params: %s", msg.params.dump().c_str());
        ESP_LOGD(TAG, "ID: %d", msg.id);
        ESP_LOGD(TAG, "Client ID: %s", msg.client_id.c_str());

        CommandType cmd_name = std::get<CommandType>(msg.command);
        ESP_LOGD(TAG, "Creating Command for %s", magic_enum::enum_name<CommandType>(cmd_name).data());

        auto it = config_map.find(msg.command);
        if (it == config_map.end())
        {
            ESP_LOGE(TAG, "Command configuration not found");
            return ESP_ERR_INVALID_ARG;
        }

        rpc_event_config_t cmd_config = it->second;
        rpc_event_config_t cmd_response_config = {response_loop, response_event_base, response_event_id};

        ESP_LOGD(FUNCTION_NAME, "Creating command");
        std::string client_id = msg.client_id;
        int id = msg.id;
        RpcCommand *command = new RpcCommand(req, msg);
        ERRCHECK_OR_DELETE_COMMAND(command->set_config(cmd_config));
        ERRCHECK_OR_DELETE_COMMAND(command->set_response_config(cmd_response_config));
        command->post();

        return ESP_OK;
    }

    ws_command_id_t cmd;
    ws_command_config_map_t config_map;
    SemaphoreHandle_t factory_mutex;

    esp_event_loop_handle_t response_loop;
    esp_event_base_t response_event_base;
    int32_t response_event_id;
    const char *TAG = "WsCommandFactory";
};