#pragma once

#include "IWsCommand.h"

class MotorControlCommand : public IWsCommand
{
public:
    explicit MotorControlCommand(
        httpd_req_t *req,
        ws_message_t msg)
        : IWsCommand(req, msg) {}

private:
    const char *TAG = "MotorControlCommand";
};