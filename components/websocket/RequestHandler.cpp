#include "RequestHandler.hpp"
#include "esp_http_server.h"

static const char *TAG = "RequestHandler";

RequestHandler::RequestHandler(
    std::shared_ptr<ResponseSender> response_sender,
    std::shared_ptr<EventManager> event_manager,
    std::shared_ptr<ClientManager> client_manager)
    : response_sender(response_sender),
      event_manager(event_manager),
      client_manager(client_manager) {}

esp_err_t RequestHandler::handle_request(httpd_req_t *req)
{
    ESP_LOGD(TAG, "RequestHandler::handle_request - Handling request");
    // ESP_LOGD(TAG, "Headers received in RequestHandler::handle_request: ");
    // log_all_headers(req);

    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = nullptr;
    std::string client_id;

    // if (!req->sess_ctx)
    // {
    //     ESP_LOGD(TAG, "Creating session context");
    //     req->sess_ctx = malloc(sizeof(ws_client_info));
    // }
    // ws_client_info *ctx_data = (ws_client_info *)req->sess_ctx;
    // ESP_RETURN_ON_FALSE(ctx_data != nullptr, ESP_ERR_NO_MEM, TAG, "Failed to allocate memory for session context");
    // if (ctx_data->client_id.empty())
    // {
    //     ctx_data->client_id = strdup(client_id.c_str());
    // }

    if (req->method == HTTP_GET)
    {
        esp_err_t ret = handle_handshake(req, client_id);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to handle handshake");
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Failed to handle handshake");
        }

        ESP_LOGD(TAG, "Handshake done, returning.");
        return ret;
    }

    ESP_RETURN_ON_ERROR(receive_frame(req, ws_pkt, buf), TAG, "Failed to receive frame");
    return process_message(req, ws_pkt, buf, client_id);
}

esp_err_t RequestHandler::identify_client(httpd_req_t *req, std::string &client_id)
{
    char *cookie = nullptr;
    size_t buf_len = httpd_req_get_hdr_value_len(req, "Cookie");
    ESP_RETURN_ON_FALSE(buf_len > 0, ESP_ERR_NOT_FOUND, TAG, "No cookie found");
    cookie = (char *)calloc(1, buf_len + 1);
    ESP_RETURN_ON_ERROR(httpd_req_get_hdr_value_str(req, "Cookie", cookie, buf_len + 1), TAG, "Failed to get cookie value");
    ESP_LOGD(TAG, "Cookie: %s", cookie);
    ESP_RETURN_ON_ERROR(get_cookie_value(cookie, "client_id", client_id), TAG, "Failed to get client_id from cookie");
    client_details_t client_details;
    return client_manager->get_client(client_id, client_details);
}

esp_err_t update_usr_ctx(httpd_req_t *req, std::string client_id)
{
    ESP_LOGD(TAG, "User context already set");

    WebSocketServer *server_instance = static_cast<WebSocketServer *>(req->user_ctx);
    ESP_RETURN_ON_FALSE(server_instance != nullptr, ESP_ERR_INVALID_ARG, TAG, "Server instance is null in user context");

    ESP_LOGD(TAG, "server instance: %p", server_instance);

    ESP_LOGD(TAG, "Getting client context from server instance");
    ws_client_info client_ctx = server_instance->get_client_ctx();
    ESP_LOGD(TAG, "client_ctx: %p", &client_ctx);

    std::string client_id_in_ctx = server_instance->get_client_id_from_ctx();
    ESP_LOGD(TAG, "Client ID in user context: %s", client_id_in_ctx.c_str());

    if (client_id_in_ctx.empty())
    {
        ESP_LOGD(TAG, "Client ID is empty in user context. Setting it to: %s", client_id.c_str());
        ESP_RETURN_ON_ERROR(server_instance->set_client_id_in_ctx(client_id), TAG, "Failed to set client ID in user context");
    }
    else
    {
        ESP_LOGD(TAG, "Client ID already set in user context: %s", client_id_in_ctx.c_str());
        ESP_RETURN_ON_FALSE(client_id_in_ctx != client_id, ESP_ERR_INVALID_ARG, TAG, "Client ID is not the same as in user context");
    }
    return ESP_OK;
}

esp_err_t RequestHandler::validate_jsonrpc2_header(httpd_req_t *req)
{
    ESP_LOGD(TAG, "Validating JSON-RPC2 header");
    const char *protocol = nullptr;
    int hdr_len = httpd_req_get_hdr_value_len(req, "Sec-WebSocket-Protocol");
    ESP_LOGD(TAG, "Sec-WebSocket-Protocol header length: %d", hdr_len);
    ESP_RETURN_ON_FALSE(hdr_len > 0, ESP_ERR_INVALID_ARG, TAG, "No Sec-WebSocket-Protocol header found");

    char *protocol_buf = (char *)malloc(hdr_len + 1);
    ESP_RETURN_ON_FALSE(protocol_buf != nullptr, ESP_ERR_NO_MEM, TAG, "Failed to allocate memory for protocol buffer");
    esp_err_t err = httpd_req_get_hdr_value_str(req, "Sec-WebSocket-Protocol", protocol_buf, hdr_len + 1);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to get Sec-WebSocket-Protocol header");

    protocol = protocol_buf;
    ESP_LOGD(TAG, "Requested Subprotocol: %s", protocol);

    if (strcmp(protocol, "jsonrpc2.0") != 0)
    {
        ESP_LOGW(TAG, "Unsupported subprotocol. Only 'jsonrpc2.0' is supported.");
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Unsupported WebSocket Subprotocol");
        return ESP_FAIL;
    }
    else
    {
        ESP_LOGD(TAG, "Subprotocol is supported: %s", protocol);
    }

    free(protocol_buf);
    return ESP_OK;
}

esp_err_t RequestHandler::handle_handshake(httpd_req_t *req, std::string client_id)
{
    ESP_LOGD(TAG, "Handling WebSocket handshake");
    ESP_RETURN_ON_FALSE(req != nullptr, ESP_ERR_INVALID_ARG, TAG, "Request is null");
    ESP_RETURN_ON_FALSE(req->handle != nullptr, ESP_ERR_INVALID_ARG, TAG, "Request handle is null");
    ESP_RETURN_ON_ERROR(validate_jsonrpc2_header(req), TAG, "Failed to validate JSON-RPC2 header");

    esp_err_t ret = identify_client(req, client_id);

    if (ret == ESP_ERR_NOT_FOUND)
    {
        ESP_LOGD(TAG, "Failed to identify client. Registering new one");
        client_id = generate_uuid();
        ESP_RETURN_ON_ERROR(client_manager->upsert_client(client_id, req->handle, req), TAG, "Failed to upsert client in client manager");
        ESP_RETURN_ON_ERROR(update_usr_ctx(req, client_id), TAG, "Failed to update user context");
    }

    httpd_resp_set_hdr(req, "Sec-WebSocket-Protocol", ws_subprotocol.c_str());
    char set_cookie[100];
    snprintf(set_cookie, sizeof(set_cookie), "client_id=%s; Path=/;", client_id.c_str());
    httpd_resp_set_hdr(req, "Set-Cookie", set_cookie);
    httpd_resp_set_hdr(req, "X-TEST-Header", "WOW... such test... very wow");
    // const char *resp_str = "WebSocket handshake response";
    // httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);

    // char handshake_response[512];
    // int len = snprintf(handshake_response, sizeof(handshake_response),
    //                    "HTTP/1.1 101 Switching Protocols\r\n"
    //                    "Upgrade: websocket\r\n"
    //                    "Connection: Upgrade\r\n"
    //                    "Sec-WebSocket-Accept: %s\r\n"
    //                    "Set-Cookie: client_id=%s; Path=/",
    //                    ws_subprotocol.c_str(), client_id.c_str());

    // // Include Sec-WebSocket-Protocol if provided
    // if (!ws_subprotocol.empty() && ws_subprotocol.length() > 0)
    // {
    //     len += snprintf(handshake_response + len, sizeof(handshake_response) - len,
    //                     "Sec-WebSocket-Protocol: %s\r\n", ws_subprotocol.c_str());
    // }
    // ESP_LOGI(TAG, "Handshake response: %s", handshake_response);

    // esp_err_t ret = httpd_resp_send(req, handshake_response, strlen(handshake_response));
    // ESP_RETURN_ON_ERROR(ret, TAG, "Failed to send handshake response");

    // const char *message = "Welcome to ESP32 WebSocket Server!";
    // ESP_LOGD(TAG, "WebSocket handshake successful");

    // httpd_ws_frame_t ws_pkt;
    // memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    // ws_pkt.payload = (uint8_t *)message;
    // ws_pkt.len = strlen(message);
    // ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    // return httpd_ws_send_frame(req, &ws_pkt);
    return ESP_OK;
}

esp_err_t RequestHandler::process_message(httpd_req_t *req, httpd_ws_frame_t &ws_pkt, uint8_t *buf, std::string client_id)
{
    ESP_LOGD(TAG, "Packet type: %d", ws_pkt.type);

    if (ws_pkt.type != HTTPD_WS_TYPE_TEXT)
    {
        ESP_LOGE(TAG, "Unsupported packet type: %d", ws_pkt.type);
        free(buf);
        return ESP_ERR_INVALID_ARG;
    }

    std::string message(reinterpret_cast<char *>(ws_pkt.payload), ws_pkt.len);
    if (message == "ping")
    {
        ESP_LOGD(TAG, "Received ping");
        free(buf);
        return handle_heartbeat(req);
    }

    ESP_LOGD(TAG, "Received message: %s", message.c_str());
    free(buf);

    ws_message_t msg;
    if (message.find("jsonrpc") != std::string::npos)
    {
        parse_json_rpc(message, msg);
    }
    else
    {
        ESP_LOGW(TAG, "Unsupported subprotocol. Only 'jsonrpc2.0' is supported.");
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Unsupported WebSocket Subprotocol");
        return ESP_FAIL;
    }

    switch (msg.command)
    {
    case ws_command_id::START_MOTORS:
        return event_manager->post_event(REMOTE_CONTROL_EVENT, remote_control_event_t::START_MOTORS);
    case ws_command_id::STOP_MOTORS:
        return event_manager->post_event(REMOTE_CONTROL_EVENT, remote_control_event_t::STOP_MOTORS);
    case ws_command_id::SET_RUNMODE:
        return event_manager->set_runlevel(msg.params, msg.id, client_id);
    default:
        ESP_LOGW(TAG, "Unknown command: %s", message.c_str());
        return ESP_ERR_INVALID_ARG;
    }
}

esp_err_t RequestHandler::handle_heartbeat(httpd_req_t *req)
{
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    ws_pkt.payload = (uint8_t *)"pong";
    ESP_LOGD(TAG, "Sending pong");
    return httpd_ws_send_frame(req, &ws_pkt);
}