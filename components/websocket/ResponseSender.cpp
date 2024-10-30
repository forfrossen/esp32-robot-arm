#include "ResponseSender.hpp"
#include "Utilities.hpp"
#include "esp_log.h"
#include <cstring>

static const char *TAG = "ResponseSender";

ResponseSender::ResponseSender(httpd_handle_t server)
    : server(server) {}

esp_err_t ResponseSender::handle_get(httpd_req_t *req, const std::string &payload) {
    // Implement your GET logic here
    cJSON *response_json = cJSON_CreateString("GET response");
    esp_err_t ret = send_response(req, httpd_req_to_sockfd(req), /*id*/ 0, response_json);
    cJSON_Delete(response_json);
    return ret;
}

esp_err_t ResponseSender::handle_get_all(httpd_req_t *req) {
    // Implement your GET_ALL logic here
    cJSON *response_json = cJSON_CreateString("GET_ALL response");
    esp_err_t ret = send_response(req, httpd_req_to_sockfd(req), /*id*/ 0, response_json);
    cJSON_Delete(response_json);
    return ret;
}

esp_err_t ResponseSender::handle_set(httpd_req_t *req, const std::string &payload) {
    // Implement your SET logic here
    cJSON *response_json = cJSON_CreateString("SET response");
    esp_err_t ret = send_response(req, httpd_req_to_sockfd(req), /*id*/ 0, response_json);
    cJSON_Delete(response_json);
    return ret;
}

esp_err_t ResponseSender::send_response(httpd_req_t *req, int client_fd, int id, const cJSON *data) {
    cJSON *response = cJSON_CreateObject();
    cJSON_AddNumberToObject(response, "id", id);
    cJSON_AddStringToObject(response, "status", "success");
    cJSON_AddItemToObject(response, "data", cJSON_Duplicate(data, true));

    char *response_str = cJSON_PrintUnformatted(response);
    AsyncRespArg *resp_arg = new AsyncRespArg{req->handle, client_fd, response_str};
    esp_err_t ret = httpd_queue_work(req->handle, ws_async_send, resp_arg);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_queue_work failed with %d", ret);
        delete resp_arg;
    }

    cJSON_Delete(response);
    free(response_str);
    return ESP_OK;
}

esp_err_t ResponseSender::send_error_response(httpd_req_t *req, int client_fd, int id, const char *error_message) {
    cJSON *response = cJSON_CreateObject();
    cJSON_AddNumberToObject(response, "id", id);
    cJSON_AddStringToObject(response, "status", "error");
    cJSON *error_data = cJSON_CreateObject();
    cJSON_AddStringToObject(error_data, "message", error_message);
    cJSON_AddItemToObject(response, "error", error_data);

    char *response_str = cJSON_PrintUnformatted(response);
    AsyncRespArg *resp_arg = new AsyncRespArg{req->handle, client_fd, response_str};
    esp_err_t ret = httpd_queue_work(req->handle, ws_async_send, resp_arg);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_queue_work failed with %d", ret);
        delete resp_arg;
    }

    cJSON_Delete(response);
    free(response_str);
    return ESP_OK;
}

void ResponseSender::ws_async_send(void *arg) {
    AsyncRespArg *resp_arg = static_cast<AsyncRespArg *>(arg);
    if (!resp_arg) {
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
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_send_frame_async failed with %d", ret);
    }

    delete resp_arg;
}

esp_err_t ResponseSender::trigger_async_send(httpd_handle_t handle, httpd_req_t *req, const std::string &data) {
    AsyncRespArg *resp_arg = new AsyncRespArg{handle, httpd_req_to_sockfd(req), data};
    esp_err_t ret = httpd_queue_work(handle, ws_async_send, resp_arg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_queue_work failed with %d", ret);
        delete resp_arg;
    }
    return ret;
}
