#ifndef WEBSOCKET_SERVER_H
#define WEBSOCKET_SERVER_H

#include "Utilities.hpp"
#include "esp_err.h"
#include "esp_http_server.h"
#include <memory>

class RequestHandler;
class ResponseSender;
class EventManager;
class ClientManager;

class WebSocketServer
{
public:
    WebSocketServer(
        std::shared_ptr<RequestHandler> request_handler,
        std::shared_ptr<ResponseSender> response_sender,
        std::shared_ptr<EventManager> event_manager,
        std::shared_ptr<ClientManager> client_manager);

    ~WebSocketServer();

    esp_err_t start();
    esp_err_t stop();

    ws_client_info get_client_ctx() { return client_ctx; }
    esp_err_t set_client_ctx(ws_client_info ctx);
    std::string get_client_id_from_ctx();
    esp_err_t set_client_id_in_ctx(std::string client_id);

private:
    httpd_handle_t server;
    std::shared_ptr<RequestHandler> request_handler;
    std::shared_ptr<ResponseSender> response_sender;
    std::shared_ptr<EventManager> event_manager;
    std::shared_ptr<ClientManager> client_manager;
    ws_client_info client_ctx;

    esp_err_t register_uri_handlers();
    static esp_err_t incoming_message_handler(httpd_req_t *req);
    static esp_err_t new_client_registration(httpd_req_t *req);
};

#endif // WEBSOCKET_SERVER_H
