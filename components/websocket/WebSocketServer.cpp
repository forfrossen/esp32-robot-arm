#include "WebSocketServer.hpp"
#include "EventManager.hpp"
#include "RequestHandler.hpp"
#include "ResponseSender.hpp"
#include <esp_log.h>
#include <memory>

static const char *TAG = "WebSocketServer";

WebSocketServer::WebSocketServer(
    std::shared_ptr<RequestHandler> request_handler,
    std::shared_ptr<ResponseSender> response_sender,
    std::shared_ptr<EventManager> event_manager,
    std::shared_ptr<ClientManager> client_manager)
    : server(nullptr),
      request_handler(request_handler),
      response_sender(response_sender),
      event_manager(event_manager),
      client_manager(client_manager) {}

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

    ESP_RETURN_ON_ERROR(httpd_start(&server, &config), TAG, "Failed to start server");

    esp_err_t ret = register_uri_handlers();
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

esp_err_t WebSocketServer::set_client_ctx(ws_client_info ctx)
{

    client_ctx = ctx;
    return ESP_OK;
}

std::string WebSocketServer::get_client_id_from_ctx()
{
    if (client_ctx.client_id.empty())
    {
        client_ctx = ws_client_info();
        client_ctx.client_id = "";
    }
    return client_ctx.client_id;
}
esp_err_t WebSocketServer::set_client_id_in_ctx(std::string client_id)
{
    if (client_ctx.client_id.empty())
    {
        client_ctx = ws_client_info();
    }
    client_ctx.client_id = client_id;
    return ESP_OK;
}

esp_err_t WebSocketServer::set_cors_headers(httpd_req_t *req)
{
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type, Authorization");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    return ESP_OK;
}

esp_err_t WebSocketServer::http_options_handler(httpd_req_t *req)
{
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type, Authorization");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    return httpd_resp_send(req, NULL, 0);
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

    // ESP_LOGD(TAG, "Received headers directly in incoming_message_handler: ");
    // log_all_headers(req);

    esp_err_t ret = server_instance->request_handler->handle_request(req);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to handle request");
    }
    return ret;
}

esp_err_t WebSocketServer::new_client_registration(httpd_req_t *req)
{
    ESP_LOGD(TAG, "New client registration");
    WebSocketServer *server_instance = static_cast<WebSocketServer *>(req->user_ctx);

    server_instance->set_cors_headers(req);

    if (req->method == HTTP_OPTIONS)
    {
        return httpd_resp_send(req, NULL, 0);
    }

    std::string client_id = generate_uuid();
    ESP_LOGD(TAG, "Generated client ID: %s", client_id.c_str());
    ESP_RETURN_ON_ERROR(server_instance->client_manager->upsert_client(client_id, req->handle, req), TAG, "Failed to upsert client");

    char set_cookie[100];
    snprintf(set_cookie, sizeof(set_cookie), "client_id=%s; Path=/;", client_id.c_str());
    httpd_resp_set_hdr(req, "Set-Cookie", set_cookie);
    httpd_resp_set_hdr(req, "X-TEST-Header", "WOW... such test... very wow");
    httpd_resp_set_status(req, "200 OK");
    httpd_resp_set_type(req, "application/json");
    std::string response = "{\"client_id\": \"" + client_id + "\"}";
    httpd_resp_send(req, response.c_str(), HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

esp_err_t WebSocketServer::api_docs_get_handler(httpd_req_t *req)
{
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type, Authorization");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    if (req->method == HTTP_OPTIONS)
    {
        return httpd_resp_send(req, NULL, 0);
    }

    std::string openapi_yaml = generate_openapi_yaml();
    httpd_resp_set_status(req, "200 OK");
    httpd_resp_set_type(req, "text/yaml");
    httpd_resp_send(req, openapi_yaml.c_str(), openapi_yaml.size());
    return ESP_OK;
}

esp_err_t WebSocketServer::register_uri_handlers()
{
    ESP_LOGD(TAG, "Registering URI handlers");
    esp_err_t ret;

    // ret = httpd_register_uri_handler(server, &options);
    // ESP_RETURN_ON_ERROR(ret, TAG, "Failed to register OPTIONS URI handler");
    std::string api_base_path_str = "/api/v1";
    std::string new_client_uri_str = api_base_path_str + "/new_client";
    std::string api_docs_uri_str = api_base_path_str + "/docs";
    std::string ws_uri_str = "/ws";

    httpd_uri_t ws_uri = {.uri = ws_uri_str.c_str(), .method = HTTP_GET, .handler = WebSocketServer::incoming_message_handler, .user_ctx = this, .is_websocket = true, .handle_ws_control_frames = false, .supported_subprotocol = "jsonrpc2.0"};

    httpd_uri_t new_client_url = {.uri = new_client_uri_str.c_str(), .method = HTTP_GET, .handler = WebSocketServer::new_client_registration, .user_ctx = this, .is_websocket = false};
    httpd_uri_t new_client_options = {.uri = new_client_uri_str.c_str(), .method = HTTP_OPTIONS, .handler = WebSocketServer::new_client_registration, .user_ctx = this, .is_websocket = false};

    httpd_uri_t api_docs_uri = {.uri = api_docs_uri_str.c_str(), .method = HTTP_GET, .handler = WebSocketServer::api_docs_get_handler, .user_ctx = this, .is_websocket = false};
    httpd_uri_t api_docs_options = {.uri = api_docs_uri_str.c_str(), .method = HTTP_OPTIONS, .handler = WebSocketServer::api_docs_get_handler, .user_ctx = this, .is_websocket = false};

    ret = httpd_register_uri_handler(server, &ws_uri);
    ESP_RETURN_ON_ERROR(ret, TAG, "Failed to register WebSocket URI handler. Uri: %s", ws_uri.uri);
    ESP_LOGD(TAG, "Registered WebSocket URI handler. Uri: %s", ws_uri.uri);

    ret = httpd_register_uri_handler(server, &new_client_options);
    ESP_RETURN_ON_ERROR(ret, TAG, "Failed to register new client URI handler. Uri: %s", new_client_options.uri);
    ESP_LOGD(TAG, "Registered new client URI handler. Uri: %s", new_client_options.uri);
    ret = httpd_register_uri_handler(server, &new_client_url);
    ESP_RETURN_ON_ERROR(ret, TAG, "Failed to register new client URI handler. Uri: %s", new_client_url.uri);
    ESP_LOGD(TAG, "Registered new client URI handler. Uri: %s", new_client_url.uri);

    ret = httpd_register_uri_handler(server, &api_docs_options);
    ESP_RETURN_ON_ERROR(ret, TAG, "Failed to register API docs URI handler. Uri: %s", api_docs_options.uri);
    ESP_LOGD(TAG, "Registered API docs URI handler. Uri: %s", api_docs_options.uri);
    ret = httpd_register_uri_handler(server, &api_docs_uri);
    ESP_RETURN_ON_ERROR(ret, TAG, "Failed to register API docs URI handler. Uri: %s", api_docs_uri.uri);
    ESP_LOGD(TAG, "Registered API docs URI handler. Uri: %s", api_docs_uri.uri);

    return ret;
}
