#ifndef WEBSOCKET_SERVER_H
#define WEBSOCKET_SERVER_H

#include "esp_http_server.h"
#include "esp_err.h"
#include "Utilities.hpp"
#include <memory>

class RequestHandler;
class ResponseSender;
class EventManager;

class WebSocketServer {
public:
    WebSocketServer(
        std::shared_ptr<RequestHandler> request_handler,
        std::shared_ptr<ResponseSender> response_sender,
        std::shared_ptr<EventManager> event_manager);

    ~WebSocketServer();

    esp_err_t start();
    esp_err_t stop();

private:
    httpd_handle_t server;
    std::shared_ptr<RequestHandler> request_handler;
    std::shared_ptr<ResponseSender> response_sender;
    std::shared_ptr<EventManager> event_manager;

    esp_err_t register_uri_handlers();
    static esp_err_t incoming_message_handler(httpd_req_t *req);
};

#endif // WEBSOCKET_SERVER_H

