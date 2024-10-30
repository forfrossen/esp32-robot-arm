#include "WebSocketServer.hpp"
#include "RequestHandler.hpp"
#include "ResponseSender.hpp"
#include "EventManager.hpp"
#include "esp_log.h"

static const char *TAG = "WebSocketServer";

WebSocketServer::WebSocketServer(
    std::shared_ptr<RequestHandler> requestHandler,
    std::shared_ptr<ResponseSender> responseSender,
    std::shared_ptr<EventManager> eventManager)
    : server(nullptr),
      requestHandler(requestHandler),
      responseSender(responseSender),
      eventManager(eventManager) {}

WebSocketServer::~WebSocketServer() {
    stop();
}

esp_err_t WebSocketServer::start() {
    if (server != nullptr) {
        ESP_LOGW(TAG, "Server already running");
        return ESP_OK;
    }

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGD(TAG, "Starting server on port: '%d'", config.server_port);
    esp_err_t ret = httpd_start(&server, &config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start server: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = register_uri_handlers();
    if (ret != ESP_OK) {
        httpd_stop(server);
        server = nullptr;
        return ret;
    }

    ESP_LOGI(TAG, "WebSocket server started");
    return ESP_OK;
}

esp_err_t WebSocketServer::stop() {
    if (server != nullptr) {
        esp_err_t ret = httpd_stop(server);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "WebSocket server stopped");
            server = nullptr;
        } else {
            ESP_LOGE(TAG, "Failed to stop server: %s", esp_err_to_name(ret));
        }
        return ret;
    }
    ESP_LOGW(TAG, "Server not running");
    return ESP_OK;
}

esp_err_t WebSocketServer::register_uri_handlers() {
    httpd_uri_t ws_uri = {
        .uri = "/ws",
        .method = HTTP_GET,
        .handler = WebSocketServer::incoming_message_handler,
        .user_ctx = this, // Pass the server instance to the handler
        .is_websocket = true,
    };

    ESP_LOGD(TAG, "Registering URI handlers");
    return httpd_register_uri_handler(server, &ws_uri);
}

esp_err_t WebSocketServer::incoming_message_handler(httpd_req_t *req) {
    WebSocketServer *server_instance = static_cast<WebSocketServer *>(req->user_ctx);
    if (!server_instance) {
        ESP_LOGE(TAG, "Server instance is null in handler");
        return ESP_ERR_INVALID_ARG;
    }

    // Delegate handling to RequestHandler
    return server_instance->requestHandler->handle_request(req);
}
