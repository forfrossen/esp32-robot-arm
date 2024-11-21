#pragma once
#ifndef RESPONSE_SENDER_H
#define RESPONSE_SENDER_H

#include "ClientManager.hpp"
#include "IWsCommand.h"
#include "Utilities.hpp"
#include "WsCommandDefs.hpp"
#include <cstring>
#include <esp_err.h>
#include <esp_http_server.h>
#include <esp_log.h>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>

class ResponseSender
{
public:
    ResponseSender(httpd_handle_t server, std::shared_ptr<ClientManager> client_manager);

    esp_err_t handle_get(httpd_req_t *req, const std::string &payload);
    esp_err_t handle_get_all(httpd_req_t *req);
    esp_err_t handle_set(httpd_req_t *req, const std::string &payload);

    esp_err_t send_rpc_response(rpc_event_data *data);

private:
    httpd_handle_t server;
    std::shared_ptr<ClientManager> client_manager;

    struct AsyncRespArg
    {
        httpd_handle_t hd;
        int fd;
        std::string data;
    };

    static void ws_async_send(void *arg);
    esp_err_t trigger_async_send(httpd_handle_t handle, httpd_req_t *req, const std::string &data);
    esp_err_t send_response(httpd_req_t *req, int client_fd, int id, const nlohmann::json &data);
    esp_err_t send_error_response(httpd_req_t *req, int client_fd, int id, const char *error_message);
};

#endif // RESPONSE_SENDER_H
