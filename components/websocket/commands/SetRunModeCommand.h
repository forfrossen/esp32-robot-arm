#pragma once

#include "IWsCommand.h"

class SetRunmodeCommand : public IWsCommand
{
public:
    explicit SetRunmodeCommand() {}

    RunMode get_run_mode() const { return run_mode; }

    esp_err_t set_run_mode(RunMode run_mode)
    {
        this->run_mode = run_mode;
        return ESP_OK;
    }

private:
    RunMode run_mode;
};