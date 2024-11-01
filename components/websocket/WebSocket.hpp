#ifndef WEB_SOCKET_H
#define WEB_SOCKET_H

#include "EventManager.hpp"
#include "RequestHandler.hpp"
#include "ResponseSender.hpp"
#include "TypeDefs.hpp"
#include "WebSocketServer.hpp"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "events.hpp"
#include "freertos/FreeRTOS.h"
#include "sdkconfig.h"
#include "utils.hpp"
#include <memory>

class WebSocket
{
public:
    WebSocket(
        esp_event_loop_handle_t system_event_loop,
        EventGroupHandle_t &system_event_group);
    ~WebSocket();

    esp_err_t start();
    esp_err_t stop();

private:
    std::shared_ptr<WsCommandFactory> command_factory;
    std::shared_ptr<RequestHandler> request_handler;
    std::shared_ptr<ResponseSender> response_sender;
    std::shared_ptr<EventManager> event_manager;
    std::shared_ptr<WebSocketServer> server;

    esp_event_loop_handle_t system_event_loop;
    EventGroupHandle_t &system_event_group;
    ws_command_config_map_t command_config_map;
};

#endif // WEB_SOCKET_H
