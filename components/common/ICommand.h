#pragma once

#include "TypeDefs.hpp"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "utils.hpp"

struct CommandEventData
{
    class ICommand *command;
};

class ICommand
{
public:
    virtual ~ICommand() = default;

    virtual void post(esp_event_loop_handle_t loop, esp_event_base_t event_base, int32_t event_id)
    {

        CommandEventData *data = new CommandEventData{this};
        esp_err_t err = esp_event_post_to(
            loop,
            event_base,
            event_id,
            data,
            sizeof(CommandEventData *),
            portMAX_DELAY);

        if (err != ESP_OK)
        {
            ESP_LOGE("ICommand", "Failed to post command to event loop");
            delete this;
        }
        ESP_LOGI("ICommand", "Command posted to event loop");
    }
};