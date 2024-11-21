#pragma once

#include "IWsCommand.h"

class SetRunLevelCommand : public IWsCommand
{
public:
    explicit SetRunLevelCommand(httpd_req_t *req, ws_message_t msg) : IWsCommand(req, msg) {}

    RunLevel get_run_level() const { return run_level; }

    esp_err_t set_run_level(nlohmann::json params)
    {
        ESP_RETURN_ON_ERROR(get_run_level_from_json(params, run_level), TAG, "Failed to get run mode from JSON");
        ESP_RETURN_ON_FALSE(run_level != RunLevel::UNKNOWN, ESP_ERR_INVALID_ARG, TAG, "Invalid run mode");
        return ESP_OK;
    }

private:
    const char *TAG = "SetRunLevelCommand";
    RunLevel run_level;
};