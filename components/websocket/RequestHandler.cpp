#include "RequestHandler.hpp"

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
    ESP_LOGI(TAG, "Headers received in RequestHandler::handle_request: ");
    log_all_headers(req);

    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = nullptr;
    std::string client_id;

    ESP_RETURN_ON_FALSE(req != nullptr, ESP_ERR_INVALID_ARG, TAG, "Request is null");
    ESP_RETURN_ON_FALSE(req->handle != nullptr, ESP_ERR_INVALID_ARG, TAG, "Request handle is null");

    esp_err_t ret = identify_client(req, client_id);

    if (ret == ESP_ERR_NOT_FOUND)
    {
        ESP_LOGI(TAG, "Failed to identify client. Registering new one");
        client_id = generate_uuid();
        client_manager->upsert_client(client_id, req->handle, req);
        std::string set_cookie = "client_id=" + client_id + "; Path=/; HttpOnly";
        httpd_resp_set_hdr(req, "Set-Cookie", set_cookie.c_str());
        std::string selected_subprotocol = "jsonrpc2.0";
        httpd_resp_set_hdr(req, "Sec-WebSocket-Protocol", selected_subprotocol.c_str());
    }

    if (!req->sess_ctx)
    {
        ESP_LOGD(TAG, "Creating session context");
        req->sess_ctx = malloc(sizeof(ws_session_t));
    }
    ws_session_t *ctx_data = (ws_session_t *)req->sess_ctx;
    ESP_RETURN_ON_FALSE(ctx_data != nullptr, ESP_ERR_NO_MEM, TAG, "Failed to allocate memory for session context");
    if (ctx_data->client_id.empty())
    {
        ctx_data->client_id = strdup(client_id.c_str());
    }

    if (req->method == HTTP_GET)
    {
        ESP_LOGD(TAG, "Handling WebSocket handshake");
        ESP_RETURN_ON_ERROR(validate_jsonrpc2_header(req), TAG, "Failed to validate JSON-RPC2 header");
        ESP_RETURN_ON_ERROR(receive_frame(req, ws_pkt, buf), TAG, "Failed to receive frame");
        return ESP_OK;
        // return handle_handshake(req);
    }

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

esp_err_t RequestHandler::validate_jsonrpc2_header(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Validating JSON-RPC2 header");
    log_all_headers(req);

    const char *protocol = nullptr;
    int hdr_len = httpd_req_get_hdr_value_len(req, "Sec-WebSocket-Protocol");
    ESP_LOGI(TAG, "Sec-WebSocket-Protocol header length: %d", hdr_len);
    ESP_RETURN_ON_FALSE(hdr_len > 0, ESP_ERR_INVALID_ARG, TAG, "No Sec-WebSocket-Protocol header found");

    char *protocol_buf = (char *)malloc(hdr_len + 1);
    ESP_RETURN_ON_FALSE(protocol_buf != nullptr, ESP_ERR_NO_MEM, TAG, "Failed to allocate memory for protocol buffer");
    esp_err_t err = httpd_req_get_hdr_value_str(req, "Sec-WebSocket-Protocol", protocol_buf, hdr_len + 1);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to get Sec-WebSocket-Protocol header");

    protocol = protocol_buf;
    ESP_LOGI(TAG, "Requested Subprotocol: %s", protocol);
    if (strcmp(protocol, "jsonrpc2.0") != 0)
    {
        ESP_LOGW(TAG, "Unsupported subprotocol. Only 'jsonrpc2.0' is supported.");
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Unsupported WebSocket Subprotocol");
        return ESP_FAIL;
    }
    free(protocol_buf);
    return ESP_OK;
}

esp_err_t RequestHandler::handle_handshake(httpd_req_t *req)
{
    const char *message = "Welcome to ESP32 WebSocket Server!";
    ESP_LOGI(TAG, "WebSocket handshake successful");

    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t *)message;
    ws_pkt.len = strlen(message);
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    return httpd_ws_send_frame(req, &ws_pkt);
}

esp_err_t RequestHandler::process_message(httpd_req_t *req, httpd_ws_frame_t &ws_pkt, uint8_t *buf, std::string client_id)
{
    ESP_LOGD(TAG, "Packet type: %d", ws_pkt.type);

    // if (ws_pkt.type != HTTPD_WS_TYPE_TEXT)
    // {
    //     ESP_LOGE(TAG, "Unsupported packet type: %d", ws_pkt.type);
    //     free(buf);
    //     return ESP_ERR_INVALID_ARG;
    // }

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