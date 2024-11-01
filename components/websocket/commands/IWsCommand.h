#pragma once

#include "TypeDefs.hpp"
#include "WsCommandDefs.hpp"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "utils.hpp"

class IWsCommand
{
public:
    virtual ~IWsCommand() = default;

    esp_err_t set_config(CommandEventConfig config)
    {
        CHECK_THAT(config.loop != nullptr);
        CHECK_THAT(config.event_base != nullptr);
        this->config = config;
        return ESP_OK;
    }

    virtual void post(
        esp_event_loop_handle_t loop,
        esp_event_base_t event_base,
        int32_t event_id)
    {
        CommandEventData data = {
            .command = this};

        esp_err_t err = esp_event_post_to(
            loop,
            event_base,
            event_id,
            &data,
            sizeof(CommandEventData),
            portMAX_DELAY);

        if (err != ESP_OK)
        {
            ESP_LOGE(FUNCTION_NAME, "Failed to post command to event loop");
            delete this;
        }
        ESP_LOGD(FUNCTION_NAME, "Command posted to event loop");
    }

private:
    CommandEventConfig config;
};