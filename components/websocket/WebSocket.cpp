#include "WebSocket.hpp"
#include "WebSocketServer.hpp"
#include "RequestHandler.hpp"
#include "ResponseSender.hpp"
#include "EventManager.hpp"
#include "esp_log.h"

static const char *TAG = "WebSocket";

WebSocket::WebSocket(
    esp_event_loop_handle_t system_event_loop,
    EventGroupHandle_t &system_event_group)
    : system_event_loop(system_event_loop),
      system_event_group(system_event_group) {
    ESP_LOGD(TAG, "WebSocket instance created");

    // Initialize submodules
    responseSender = std::make_shared<ResponseSender>(nullptr); // Server will be set later
    eventManager = std::make_shared<EventManager>(system_event_loop, system_event_group);
    requestHandler = std::make_shared<RequestHandler>(responseSender, eventManager);

    server = std::make_shared<WebSocketServer>(requestHandler, responseSender, eventManager);
}

WebSocket::~WebSocket() {
    ESP_LOGD(TAG, "WebSocket instance destroyed");
    stop();
}

esp_err_t WebSocket::start() {
    ESP_LOGD(TAG, "Starting WebSocket");
    esp_err_t ret = eventManager->register_handlers();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register event handlers");
        return ret;
    }

    ret = server->start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start WebSocket server");
        return ret;
    }

    xEventGroupSetBits(system_event_group, WEBSOCKET_READY);
    return ESP_OK;
}

esp_err_t WebSocket::stop() {
    ESP_LOGD(TAG, "Stopping WebSocket");
    esp_err_t ret = server->stop();
    if (ret == ESP_OK) {
        eventManager->unregister_handlers();
        xEventGroupClearBits(system_event_group, WEBSOCKET_READY);
    }
    return ret;
}

