#include "WebSocketServer.hpp"
static const char *TAG = "WebSocket";

// Initialize the static URI handler
const httpd_uri_t WebSocket::ws_uri = {
    .uri = "/ws",
    .method = HTTP_GET,
    .handler = WebSocket::incoming_message_handler,
    .user_ctx = nullptr,
    .is_websocket = true,
};

WebSocket::WebSocket(
    httpd_handle_t server,
    esp_event_loop_handle_t system_event_loop,
    EventGroupHandle_t &system_event_group)
    : server(server),
      system_event_loop(system_event_loop),
      system_event_group(system_event_group)
{
    ESP_LOGD(TAG, "WebSocket instance created");
    assert(start() == ESP_OK);
}

WebSocket::~WebSocket()
{
    ESP_LOGD(TAG, "WebSocket instance destroyed");
    stop();
}

esp_err_t WebSocket::start()
{
    if (server != nullptr)
    {
        ESP_LOGW(TAG, "Server already running");
        return ESP_OK;
    }

    server = start_webserver();
    if (server == nullptr)
    {
        ESP_LOGE(TAG, "Failed to start WebSocket server");
        return ESP_FAIL;
    }

    register_event_handlers();
    ESP_LOGD(TAG, "WebSocket server started");
    xEventGroupSetBits(system_event_group, WEBSOCKET_READY);
    return ESP_OK;
}

esp_err_t WebSocket::stop()
{
    if (server == nullptr)
    {
        ESP_LOGW(TAG, "Server not running");
        return ESP_OK;
    }

    unregister_event_handlers();
    esp_err_t ret = stop_webserver();
    if (ret == ESP_OK)
    {
        server = nullptr;
        ESP_LOGD(TAG, "WebSocket server stopped");
        xEventGroupClearBits(system_event_group, WEBSOCKET_READY);
    }
    else
    {
        ESP_LOGE(TAG, "Failed to stop WebSocket server");
    }
    return ret;
}

httpd_handle_t WebSocket::start_webserver()
{
    httpd_handle_t server = nullptr;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Assign the instance pointer to user_ctx for handlers to access
    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGD(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK)
    {
        // Update the ws_uri's user_ctx to point to this instance
        httpd_uri_t uri = ws_uri;
        uri.user_ctx = this;

        // Registering the ws handler
        ESP_LOGD(TAG, "Registering URI handlers");
        if (httpd_register_uri_handler(server, &uri) != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to register URI handler");
            httpd_stop(server);
            return nullptr;
        }
        return server;
    }

    ESP_LOGE(TAG, "Error starting server!");
    return nullptr;
}

esp_err_t WebSocket::stop_webserver()
{
    if (server != nullptr)
    {
        return httpd_stop(server);
    }
    return ESP_OK;
}

esp_err_t WebSocket::register_event_handlers()
{
    // Example: Register connect and disconnect handlers
    // You need to define SYSTEM_EVENTS and other relevant event bases as per your application

    // Replace SYSTEM_EVENTS and related constants with actual event bases and IDs
    // Example:
    // esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, this, NULL);
    // esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, this, NULL);

    // Register property change event handler
    // esp_event_handler_instance_register(PROPERTY_CHANGE_EVENT_BASE, PROPERTY_CHANGE_EVENT_ID, &property_change_event_handler, this, NULL);

    ESP_RETURN_ON_ERROR(
        esp_event_handler_register(
            WIFI_EVENT,
            WIFI_EVENT_STA_DISCONNECTED,
            &disconnect_handler,
            this),
        TAG,
        "Failed to register disconnect event handler");

    ESP_RETURN_ON_ERROR(
        esp_event_handler_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &connect_handler,
            this),
        TAG,
        "Failed to register connect event handler");
    CHECK_THAT(system_event_loop != nullptr);

    ESP_RETURN_ON_ERROR(
        esp_event_handler_register_with(
            system_event_loop,
            SYSTEM_EVENTS,
            PROPERTY_CHANGE_EVENT,
            &property_change_event_handler,
            this),
        TAG,
        "Failed to register property change event handler");
    return ESP_OK;
}

esp_err_t WebSocket::unregister_event_handlers()
{
    // Unregister event handlers similarly to how they were registered
    // Example:
    // esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler);
    // esp_event_handler_instance_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler);

    // Unregister property change event handler
    // esp_event_handler_instance_unregister(PROPERTY_CHANGE_EVENT_BASE, PROPERTY_CHANGE_EVENT_ID, &property_change_event_handler);
    return ESP_OK;
}

esp_err_t WebSocket::send_response(httpd_req_t *req, int client_fd, int id, const cJSON *data)
{
    cJSON *response = cJSON_CreateObject();
    cJSON_AddNumberToObject(response, "id", id);
    cJSON_AddStringToObject(response, "status", "success");
    cJSON_AddItemToObject(response, "data", cJSON_Duplicate(data, true));

    char *response_str = cJSON_PrintUnformatted(response);
    AsyncRespArg *resp_arg = new AsyncRespArg{req->handle, client_fd, response_str};
    httpd_queue_work(req->handle, ws_async_send, resp_arg);

    cJSON_Delete(response);
    free(response_str);
    return ESP_OK;
}

esp_err_t WebSocket::send_error_response(httpd_req_t *req, int client_fd, int id, const char *error_message)
{
    cJSON *response = cJSON_CreateObject();
    cJSON_AddNumberToObject(response, "id", id);
    cJSON_AddStringToObject(response, "status", "error");
    cJSON *error_data = cJSON_CreateObject();
    cJSON_AddStringToObject(error_data, "message", error_message);
    cJSON_AddItemToObject(response, "error", error_data);

    char *response_str = cJSON_PrintUnformatted(response);
    AsyncRespArg *resp_arg = new AsyncRespArg{req->handle, client_fd, response_str};
    httpd_queue_work(req->handle, ws_async_send, resp_arg);

    cJSON_Delete(response);
    free(response_str);
    return ESP_OK;
}

void WebSocket::ws_async_send(void *arg)
{
    AsyncRespArg *resp_arg = static_cast<AsyncRespArg *>(arg);
    if (resp_arg == nullptr)
    {
        ESP_LOGE(TAG, "AsyncRespArg is null");
        return;
    }

    httpd_handle_t hd = resp_arg->hd;
    int fd = resp_arg->fd;
    const char *data = resp_arg->data.c_str();

    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t *)data;
    ws_pkt.len = strlen(data);
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;

    esp_err_t ret = httpd_ws_send_frame_async(hd, fd, &ws_pkt);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "httpd_ws_send_frame_async failed with %d", ret);
    }

    delete resp_arg;
}

esp_err_t WebSocket::trigger_async_send(httpd_handle_t handle, httpd_req_t *req, const std::string &data)
{
    AsyncRespArg *resp_arg = new AsyncRespArg;
    if (resp_arg == nullptr)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for AsyncRespArg");
        return ESP_ERR_NO_MEM;
    }
    resp_arg->hd = handle;
    resp_arg->fd = httpd_req_to_sockfd(req);
    resp_arg->data = data;

    esp_err_t ret = httpd_queue_work(handle, ws_async_send, resp_arg);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "httpd_queue_work failed with %d", ret);
        delete resp_arg;
    }
    return ret;
}

esp_err_t WebSocket::incoming_message_handler(httpd_req_t *req)
{
    WebSocket *ws_instance = static_cast<WebSocket *>(req->user_ctx);

    // Handle the WebSocket handshake
    if (req->method == HTTP_GET)
    {
        ESP_RETURN_ON_FALSE(
            ws_instance,
            ESP_ERR_INVALID_ARG,
            TAG,
            "WebSocket instance is null in handshake");

        return ws_instance->handle_handshake(req);
    }

    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = nullptr;
    esp_err_t ret = ws_instance->receive_frame(req, ws_pkt, buf);
    ESP_RETURN_ON_ERROR(ret, TAG, "Failed to receive frame");
    return ws_instance->process_message(req, ws_pkt, buf);
}

esp_err_t WebSocket::process_message(httpd_req_t *req, httpd_ws_frame_t &ws_pkt, uint8_t *buf)
{
    ESP_LOGD(TAG, "Packet type: %d", ws_pkt.type);

    if (ws_pkt.type != HTTPD_WS_TYPE_TEXT)
    {
        ESP_LOGW(TAG, "Unsupported packet type: %d", ws_pkt.type);
        free(buf);
        return ESP_ERR_INVALID_ARG;
    }

    std::string message(reinterpret_cast<char *>(ws_pkt.payload), ws_pkt.len);
    ESP_LOGD(TAG, "Received message: %s", message.c_str());
    free(buf);

    ws_message_t msg;
    ESP_RETURN_ON_ERROR(split_ws_msg(message, msg), TAG, "Failed to split message");
    ws_command_t command = msg.first;
    ws_payload_t payload = msg.second;

    WebSocket *ws_instance = static_cast<WebSocket *>(req->user_ctx);
    ESP_RETURN_ON_FALSE(
        ws_instance,
        ESP_ERR_INVALID_ARG,
        TAG,
        "WebSocket instance is null in process_message");

    esp_err_t ret = ESP_OK;

    if (command == "GET")
    {
        ret = ws_instance->handle_get(req, payload);
    }
    else if (command == "GET_ALL")
    {
        ret = ws_instance->handle_get_all(req);
    }
    else if (command == "SET")
    {
        ret = ws_instance->handle_set(req, payload);
    }
    else if (command == "START")
    {
        ret = ws_instance->post_system_event(REMOTE_CONTROL_EVENT, remote_control_event_t::START_MOTORS);
    }
    else if (command == "STOP")
    {
        ret = ws_instance->post_system_event(REMOTE_CONTROL_EVENT, remote_control_event_t::STOP_MOTORS);
    }
    else if (command == "SET_RUNLEVEL")
    {
        ESP_LOGI(TAG, "Setting runlevel: %s", payload.c_str());
        switch (std::stoi(payload))
        {
        case 0:
            xEventGroupSetBits(system_event_group, RUNLEVEL_0);
            xEventGroupClearBits(system_event_group, RUNLEVEL_1 | RUNLEVEL_2 | RUNLEVEL_3);
            trigger_async_send(req->handle, req, "SUCCESS");
            break;
        case 1:
            xEventGroupSetBits(system_event_group, RUNLEVEL_1 | RUNLEVEL_0);
            xEventGroupClearBits(system_event_group, RUNLEVEL_2 | RUNLEVEL_3);
            trigger_async_send(req->handle, req, "SUCCESS");
            break;
        case 2:
            xEventGroupSetBits(system_event_group, RUNLEVEL_2 | RUNLEVEL_1 | RUNLEVEL_0);
            xEventGroupClearBits(system_event_group, RUNLEVEL_3);
            trigger_async_send(req->handle, req, "SUCCESS");
            break;
        case 3:
            xEventGroupSetBits(system_event_group, RUNLEVEL_3 | RUNLEVEL_2 | RUNLEVEL_1 | RUNLEVEL_0);
            trigger_async_send(req->handle, req, "SUCCESS");
            break;
        default:
            ESP_LOGW(TAG, "Invalid runlevel: %s", payload.c_str());
            trigger_async_send(req->handle, req, "ERROR");
            ret = ESP_ERR_INVALID_ARG;
        }
    }
    else
    {
        ESP_LOGW(TAG, "Unknown command: %s", message.c_str());
        ret = ESP_ERR_INVALID_ARG;
    }

    ESP_RETURN_ON_ERROR(ret, TAG, "Failed to process message: %s", message.c_str());
    return ret;
}

esp_err_t WebSocket::handle_get(httpd_req_t *req, ws_payload_t &message)
{
    // Implement your GET logic here
    const char *response = "GET response";
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t *)response;
    ws_pkt.len = strlen(response);
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    return httpd_ws_send_frame(req, &ws_pkt);
}

esp_err_t WebSocket::handle_get_all(httpd_req_t *req)
{
    // Implement your GET_ALL logic here
    const char *response = "GET_ALL response";
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t *)response;
    ws_pkt.len = strlen(response);
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    return httpd_ws_send_frame(req, &ws_pkt);
}

esp_err_t WebSocket::handle_set(httpd_req_t *req, ws_payload_t &message)
{
    // Implement your SET logic here
    const char *response = "SET response";
    httpd_ws_frame_t ws_pkt_response;
    memset(&ws_pkt_response, 0, sizeof(httpd_ws_frame_t));
    ws_pkt_response.payload = (uint8_t *)response;
    ws_pkt_response.len = strlen(response);
    ws_pkt_response.type = HTTPD_WS_TYPE_TEXT;
    return httpd_ws_send_frame(req, &ws_pkt_response);
}

esp_err_t WebSocket::receive_frame(httpd_req_t *req, httpd_ws_frame_t &ws_pkt, uint8_t *&buf)
{
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;

    // Set max_len = 0 to get the frame len
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    ESP_RETURN_ON_ERROR(ret, TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
    ESP_LOGD(TAG, "Frame length is %d", ws_pkt.len);
    ESP_RETURN_ON_FALSE(ws_pkt.len, ESP_OK, TAG, "Frame length is 0");
    // ws_pkt.len + 1 is for NULL termination as we are expecting a string
    buf = (uint8_t *)calloc(1, ws_pkt.len + 1);
    ESP_RETURN_ON_FALSE(buf != nullptr, ESP_ERR_NO_MEM, TAG, "Failed to allocate memory for buffer");
    ws_pkt.payload = buf;

    // Set max_len = ws_pkt.len to get the frame payload
    ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
        free(buf);
        return ret;
    }
    ESP_LOGD(TAG, "Received packet with message: %s", ws_pkt.payload);

    return ESP_OK;
}

esp_err_t WebSocket::handle_handshake(httpd_req_t *req)
{
    WebSocket *ws_instance = static_cast<WebSocket *>(req->user_ctx);
    ESP_RETURN_ON_FALSE(
        ws_instance,
        ESP_ERR_INVALID_ARG,
        TAG,
        "WebSocket instance is null in handshake");

    ESP_LOGD(TAG, "Handshake done, the new connection was opened");

    // Send a welcome message
    const char *message = "Welcome to ESP32 WebSocket Server!";
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t *)message;
    ws_pkt.len = strlen(message);
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    return httpd_ws_send_frame(req, &ws_pkt);
}

void WebSocket::disconnect_handler(void *arg, esp_event_base_t event_base,
                                   int32_t event_id, void *event_data)
{
    WebSocket *ws_instance = static_cast<WebSocket *>(arg);
    if (ws_instance)
    {
        ESP_LOGD(TAG, "Stopping webserver due to disconnect event");
        ws_instance->stop();
    }
    else
    {
        ESP_LOGE(TAG, "WebSocket instance is null in disconnect_handler");
    }
}

void WebSocket::connect_handler(void *arg, esp_event_base_t event_base,
                                int32_t event_id, void *event_data)
{
    WebSocket *ws_instance = static_cast<WebSocket *>(arg);
    if (ws_instance)
    {
        ESP_LOGD(TAG, "Starting webserver due to connect event");
        ws_instance->start();
    }
    else
    {
        ESP_LOGE(TAG, "WebSocket instance is null in connect_handler");
    }
}

void WebSocket::property_change_event_handler(void *args, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    WebSocket *ws_instance = static_cast<WebSocket *>(args);
    if (ws_instance)
    {
        ESP_LOGD(TAG, "GOT PROPERTY CHANGE EVENT, EVENT_BASE: %s, EVENT_ID: %ld", event_base, event_id);

        if (strcmp(event_base, "SYSTEM_EVENTS") != 0 || event_id != /* PROPERTY_CHANGE_EVENT_ID */ 0)
        {
            return;
        }

        // Example: Send a property change notification to all connected clients
        // You need to implement a mechanism to keep track of connected clients
        // and iterate over them to send messages.

        // const char *message = "Property has changed!";
        // httpd_ws_frame_t ws_pkt;
        // memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
        // ws_pkt.payload = (uint8_t *)message;
        // ws_pkt.len = strlen(message);
        // ws_pkt.type = HTTPD_WS_TYPE_TEXT;
        // httpd_ws_send_frame_to_all(ws_instance->server_, &ws_pkt);
    }
}

esp_err_t WebSocket::post_system_event(system_event_id_t event, remote_control_event_t message)
{
    ESP_LOGD(TAG, "Posting system event %s with message: %s", magic_enum::enum_name(event).data(), magic_enum::enum_name(message).data());
    CHECK_THAT(system_event_loop != nullptr);

    esp_err_t ret = esp_event_post_to(
        system_event_loop,
        SYSTEM_EVENTS,
        event,
        static_cast<const void *>(&message),
        sizeof(remote_control_event_t),
        portMAX_DELAY);

    ESP_RETURN_ON_ERROR(
        ret,
        TAG,
        "Error posting system event");
    return ESP_OK;
}