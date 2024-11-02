#ifndef RESPONSE_SENDER_H
#define RESPONSE_SENDER_H

// #include "..\managed_components\johboh__nlohmann-json\single_include\nlohmann\json.hpp"
#include "../../managed_components/johboh__nlohmann-json/single_include/nlohmann/json.hpp"
#include "Utilities.hpp"
#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include <cstring>
#include <memory>
#include <string>

class ResponseSender
{
public:
    ResponseSender(httpd_handle_t server);

    esp_err_t handle_get(httpd_req_t *req, const std::string &payload);
    esp_err_t handle_get_all(httpd_req_t *req);
    esp_err_t handle_set(httpd_req_t *req, const std::string &payload);

private:
    httpd_handle_t server;

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

#endif \\ RESPONSE_SENDER_H
