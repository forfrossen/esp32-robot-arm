#pragma once

#include "TypeDefs.hpp"
#include "Utilities.hpp"
#include "WsCommandDefs.hpp"
#include "utils.hpp"
#include <esp_err.h>
#include <esp_event.h>
#include <esp_log.h>
#include <nlohmann/json.hpp>

class IWsCommand
{
public:
    IWsCommand(httpd_req_t *req, ws_message_t msg) : req(req), msg(msg) {}
    virtual ~IWsCommand() {}

    ws_message_t get_msg() const { return msg; }
    httpd_req_t *get_req() const { return req; }

    void set_result(bool result) { this->result = result; }
    bool get_result() { return result; }
    int get_id() { return msg.id; }
    std::string get_client_id() { return msg.client_id; }

    esp_err_t set_config(rpc_event_config_t config)
    {
        CHECK_THAT(config.loop != nullptr);
        CHECK_THAT(config.event_base != nullptr);
        this->config = config;
        return ESP_OK;
    }

    esp_err_t set_response_config(rpc_event_config_t response_config)
    {
        CHECK_THAT(response_config.loop != nullptr);
        CHECK_THAT(response_config.event_base != nullptr);
        this->response_config = response_config;
        return ESP_OK;
    }

    esp_err_t post()
    {
        data = {.command = this};

        ERRCHECK_OR_DELETE_THIS(
            esp_event_post_to(
                config.loop,
                config.event_base,
                config.event_id,
                &data,
                sizeof(rpc_event_data),
                portMAX_DELAY));

        ESP_LOGD(FUNCTION_NAME, "Command posted to event loop");
        return ESP_OK;
    }

    esp_err_t post_result(bool result)
    {
        this->result = result;
        data = {.command = this};

        ERRCHECK_OR_DELETE_THIS(
            esp_event_post_to(
                response_config.loop,
                response_config.event_base,
                response_config.event_id,
                &data,
                sizeof(rpc_event_data),
                portMAX_DELAY));

        ESP_LOGD(FUNCTION_NAME, "Response posted to event loop");
        return ESP_OK;
    }

private:
    rpc_event_config_t config;
    rpc_event_config_t response_config;
    rpc_event_data data;
    httpd_req_t *req;
    ws_message_t msg;

    bool result;
    const char *TAG = "IWsCommand";
};