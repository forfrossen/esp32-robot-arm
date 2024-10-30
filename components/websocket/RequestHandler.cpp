#include "RequestHandler.hpp"
#include "EventManager.hpp"
#include "ResponseSender.hpp"
#include "Utilities.hpp"
#include "cJSON.h"
#include "esp_log.h"
#include <cstring>

static const char *TAG = "RequestHandler";

RequestHandler::RequestHandler(
    std::shared_ptr<ResponseSender> responseSender,
    std::shared_ptr<EventManager> eventManager)
    : responseSender(responseSender),
      eventManager(eventManager) {}

esp_err_t RequestHandler::handle_request(httpd_req_t *req)
{
    if (req->method == HTTP_GET)
    {
        ESP_LOGD(TAG, "Handling WebSocket handshake");
        return handle_handshake(req);
    }

    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = nullptr;
    esp_err_t ret = receive_frame(req, ws_pkt, buf);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to receive frame");
        return ret;
    }

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
        ESP_LOGW(TAG, "Unsupported packet type: %d", ws_pkt.type);
        free(buf);
        return ESP_ERR_INVALID_ARG;
    }

    std::string message(reinterpret_cast<char *>(ws_pkt.payload), ws_pkt.len);
    ESP_LOGD(TAG, "Received message: %s", message.c_str());
    free(buf);

    ws_message_t msg;
    esp_err_t ret = parse_json_msg(message, msg);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to split message");
        return ret;
    }

    ws_command_t command = msg.first;
    ws_payload_t payload = msg.second;

    if (command == "GET")
    {
        return responseSender->handle_get(req, payload);
    }
    else if (command == "GET_ALL")
    {
        return responseSender->handle_get_all(req);
    }
    else if (command == "SET")
    {
        return responseSender->handle_set(req, payload);
    }
    else if (command == "START")
    {
        return eventManager->post_event(REMOTE_CONTROL_EVENT, remote_control_event_t::START_MOTORS);
    }
    else if (command == "STOP")
    {
        return eventManager->post_event(REMOTE_CONTROL_EVENT, remote_control_event_t::STOP_MOTORS);
    }
    else if (command == "SET_RUNLEVEL")
    {
        ESP_LOGI(TAG, "Setting runlevel: %s", payload.c_str());
        int runlevel = std::stoi(payload);
        esp_err_t run_ret = eventManager->set_runlevel(runlevel, req);
        if (run_ret != ESP_OK)
        {
            ESP_LOGW(TAG, "Invalid runlevel: %s", payload.c_str());
            return run_ret;
        }
        return ESP_OK;
    }
    else
    {
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
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
        return ret;
    }

    ESP_LOGD(TAG, "Frame length is %d", ws_pkt.len);
    if (ws_pkt.len == 0)
    {
        ESP_LOGW(TAG, "Frame length is 0");
        return ESP_ERR_INVALID_ARG;
    }

    buf = (uint8_t *)calloc(1, ws_pkt.len + 1); // +1 for NULL termination
    if (!buf)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for buffer");
        return ESP_ERR_NO_MEM;
    }

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
