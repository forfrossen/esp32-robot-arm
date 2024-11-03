#pragma once

#include "IWsCommand.h"

class RpcCommand : public IWsCommand
{
public:
    explicit RpcCommand() {}

    nlohmann::json get_params() const { return params; }

    esp_err_t set_params(nlohmann::json params)
    {
        this->params = params;
        return ESP_OK;
    }

private:
    const char *TAG = "RpcCommand";
    nlohmann::json params;
};