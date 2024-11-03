#pragma once

#include "../../managed_components/johboh__nlohmann-json/single_include/nlohmann/json.hpp"
#include "TypeDefs.hpp"
#include "Utilities.hpp"
#include "WsCommandDefs.hpp"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "utils.hpp"

class IWsCommand
{
public:
    explicit IWsCommand(int id, std::string client_id) : id(id), client_id(client_id) {}
    virtual ~IWsCommand() = default;
    void set_result(bool result) { this->result = result; }
    bool get_result() { return result; }
    int get_id() { return id; }
    std::string get_client_id() { return client_id; }

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
    int id;
    std::string client_id;
    rpc_event_config_t config;
    rpc_event_config_t response_config;
    rpc_event_data data;
    bool result;
    const char *TAG = "IWsCommand";
};