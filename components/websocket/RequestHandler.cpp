#include "RequestHandler.hpp"
#include "EventManager.hpp"
#include "ResponseSender.hpp"
#include "Utilities.hpp"
#include "cJSON.h"
#include "esp_log.h"
#include <cstring>

static const char *TAG = "RequestHandler";

RequestHandler::RequestHandler(
    std::shared_ptr<ResponseSender> response_sender,
    std::shared_ptr<EventManager> event_manager)
    : response_sender(response_sender),
      event_manager(event_manager) {}

esp_err_t RequestHandler::handle_request(httpd_req_t *req)
{
    if (req->method == HTTP_GET)
    {
        ESP_LOGD(TAG, "Handling WebSocket handshake");
        // return handle_handshake(req);
        return ESP_OK;
    }

    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = nullptr;

    ESP_RETURN_ON_ERROR(receive_frame(req, ws_pkt, buf), TAG, "Failed to receive frame");

    return process_message(req, ws_pkt, buf);
}

esp_err_t RequestHandler::handle_handshake(httpd_req_t *req)
{
    // Send a welcome message
    const char *message = "Welcome to ESP32 WebSocket Server!";
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t *)message;
    ws_pkt.len = strlen(message);
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;

    return httpd_ws_send_frame(req, &ws_pkt);
}

esp_err_t RequestHandler::process_message(httpd_req_t *req, httpd_ws_frame_t &ws_pkt, uint8_t *buf)
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
        return handle_heartbeat(req);
    }

    ESP_LOGD(TAG, "Received message: %s", message.c_str());
    free(buf);

    ws_message_t msg = std::make_pair(ws_command_id::UNKNOWN, ws_payload_t{});
    esp_err_t ret = parse_json_msg(message, msg);
    ESP_RETURN_ON_ERROR(parse_json_msg(message, msg), TAG, "Failed to parse JSON message");

    ws_command_id command = msg.first;
    ws_payload_t payload = msg.second;

    switch (command)
    {
    case ws_command_id::START_MOTORS:
        return event_manager->post_event(REMOTE_CONTROL_EVENT, remote_control_event_t::START_MOTORS);
    case ws_command_id::STOP_MOTORS:
        return event_manager->post_event(REMOTE_CONTROL_EVENT, remote_control_event_t::STOP_MOTORS);
    case ws_command_id::SET_RUNMODE:
        return event_manager->set_runlevel(payload);
    default:
        ESP_LOGW(TAG, "Unknown command: %s", message.c_str());
        return ESP_ERR_INVALID_ARG;
    }
}

esp_err_t RequestHandler::receive_frame(httpd_req_t *req, httpd_ws_frame_t &ws_pkt, uint8_t *&buf)
{
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;

    // Get frame length
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    ESP_RETURN_ON_ERROR(ret, TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
    ESP_LOGD(TAG, "Frame length is %d", ws_pkt.len);
    ESP_RETURN_ON_FALSE(ws_pkt.len > 0, ESP_ERR_INVALID_ARG, TAG, "Frame length is 0");

    buf = (uint8_t *)calloc(1, ws_pkt.len + 1); // +1 for NULL termination

    ESP_RETURN_ON_FALSE(buf != nullptr, ESP_ERR_NO_MEM, TAG, "Failed to allocate memory for buffer");

    ws_pkt.payload = buf;

    // Receive actual payload
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

esp_err_t RequestHandler::handle_heartbeat(httpd_req_t *req)
{
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    ws_pkt.payload = (uint8_t *)"pong";
    ESP_LOGD(TAG, "Sending pong");
    return httpd_ws_send_frame(req, &ws_pkt);
}