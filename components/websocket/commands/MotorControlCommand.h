#pragma once

#include "IWsCommand.h"

class MotorControlCommand : public IWsCommand
{
public:
    explicit MotorControlCommand(
        int id,
        std::string client_id,
        httpd_req_t *req,
        ws_message_t msg)
        : IWsCommand(id, client_id),
          req(req),
          msg(msg) {}

    ws_message_t get_msg() const { return msg; }

private:
    const char *TAG = "MotorControlCommand";
    httpd_req_t *req;
    ws_message_t msg;
};