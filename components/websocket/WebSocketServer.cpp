#include "WebSocketServer.hpp"
#include "EventManager.hpp"
#include "RequestHandler.hpp"
#include "ResponseSender.hpp"
#include "esp_log.h"

static const char *TAG = "WebSocketServer";

WebSocketServer::WebSocketServer(
    std::shared_ptr<RequestHandler> request_handler,
    std::shared_ptr<ResponseSender> response_sender,
    std::shared_ptr<EventManager> event_manager)
    : server(nullptr),
      request_handler(request_handler),
      response_sender(response_sender),
      event_manager(event_manager) {}

WebSocketServer::~WebSocketServer()
{
    stop();
}

esp_err_t WebSocketServer::start()
{
    ESP_RETURN_ON_FALSE(event_manager != nullptr, ESP_FAIL, TAG, "Event manager is null");
    ESP_RETURN_ON_FALSE(request_handler != nullptr, ESP_FAIL, TAG, "Request handler is null");
    ESP_RETURN_ON_FALSE(response_sender != nullptr, ESP_FAIL, TAG, "Response sender is null");
    ESP_RETURN_ON_FALSE(server == nullptr, ESP_FAIL, TAG, "Server already running");

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    // config.enable_so_linger = true;
    config.keep_alive_enable = true;
    config.keep_alive_idle = 60;
    config.keep_alive_interval = 5;
    config.keep_alive_count = 3;

    ESP_LOGD(TAG, "Starting server on port: '%d'", config.server_port);
    esp_err_t ret = httpd_start(&server, &config);
    ESP_RETURN_ON_ERROR(ret, TAG, "Failed to start server");

    ret = register_uri_handlers();
    if (ret != ESP_OK)
    {
        httpd_stop(server);
        server = nullptr;
        return ret;
    }

    ESP_LOGI(TAG, "WebSocket server started");
    return ESP_OK;
}

esp_err_t WebSocketServer::stop()
{
    if (server != nullptr)
    {
        esp_err_t ret = httpd_stop(server);
        if (ret == ESP_OK)
        {
            ESP_LOGI(TAG, "WebSocket server stopped");
            server = nullptr;
        }
        else
        {
            ESP_LOGE(TAG, "Failed to stop server: %s", esp_err_to_name(ret));
        }
        return ret;
    }
    ESP_LOGW(TAG, "Server not running");
    return ESP_OK;
}

esp_err_t WebSocketServer::incoming_message_handler(httpd_req_t *req)
{
    WebSocketServer *server_instance = static_cast<WebSocketServer *>(req->user_ctx);
    if (!server_instance)
    {
        ESP_LOGE(TAG, "Server instance is null in handler");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGD(TAG, "Received message in incoming_message_handler. Method is: %d", req->method);

    ESP_LOGD(TAG, "Received headers directly in incoming_message_handler: ");
    log_all_headers(req);

    esp_err_t ret = server_instance->request_handler->handle_request(req);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to handle request");
    }
    return ret;
}

esp_err_t WebSocketServer::register_uri_handlers()
{
    httpd_uri_t ws_uri = {
        .uri = "/ws",
        .method = HTTP_GET,
        .handler = WebSocketServer::incoming_message_handler,
        .user_ctx = this,
        .is_websocket = true,
        .handle_ws_control_frames = false,
        .supported_subprotocol = "jsonrpc2.0"};

    ESP_LOGD(TAG, "Registering URI handlers");
    return httpd_register_uri_handler(server, &ws_uri);
}