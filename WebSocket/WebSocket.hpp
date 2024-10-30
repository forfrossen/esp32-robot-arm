#ifndef WEB_SOCKET_H
#define WEB_SOCKET_H

#include "esp_event.h"
#include "esp_err.h"
#include "esp_http_server.h"
#include "freertos/FreeRTOS.h"
#include "sdkconfig.h"
#include "TypeDefs.hpp"
#include "events.hpp"
#include "utils.hpp"

#include <memory>

#include "WebSocketServer.hpp"
#include "RequestHandler.hpp"
#include "ResponseSender.hpp"
#include "EventManager.hpp"

class WebSocket {
public:
    WebSocket(
        esp_event_loop_handle_t system_event_loop,
        EventGroupHandle_t &system_event_group);
    ~WebSocket();

    esp_err_t start();
    esp_err_t stop();

private:
    std::shared_ptr<RequestHandler> requestHandler;
    std::shared_ptr<ResponseSender> responseSender;
    std::shared_ptr<EventManager> eventManager;
    std::shared_ptr<WebSocketServer> server;

    esp_event_loop_handle_t system_event_loop;
    EventGroupHandle_t &system_event_group;
};

#endif // WEB_SOCKET_H

