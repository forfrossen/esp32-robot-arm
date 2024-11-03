#pragma once

#include "IWsCommand.h"

class SetRunLevelCommand : public IWsCommand
{
public:
    explicit SetRunLevelCommand(int id, std::string client_id) : IWsCommand(id, client_id) {}

    RunLevel get_run_level() const { return run_level; }

    esp_err_t set_run_level(nlohmann::json run_level)
    {
        ESP_RETURN_ON_ERROR(get_run_level_from_json(run_level, this->run_level), TAG, "Failed to get run mode from JSON");
        return ESP_OK;
    }

private:
    const char *TAG = "SetRunLevelCommand";
    RunLevel run_level;
};