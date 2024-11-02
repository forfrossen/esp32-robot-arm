#include "ResponseSender.hpp"

static const char *TAG = "ResponseSender";

ResponseSender::ResponseSender(httpd_handle_t server)
    : server(server) {}

esp_err_t ResponseSender::handle_get(httpd_req_t *req, const std::string &payload)
{
    // Implement your GET logic here
    nlohmann::json response_json = "GET response";
    esp_err_t ret = send_response(req, httpd_req_to_sockfd(req), /*id*/ 0, response_json);
    return ret;
}

esp_err_t ResponseSender::handle_get_all(httpd_req_t *req)
{
    // Implement your GET_ALL logic here
    nlohmann::json response_json = "GET_ALL response";
    esp_err_t ret = send_response(req, httpd_req_to_sockfd(req), /*id*/ 0, response_json);
    return ret;
}

esp_err_t ResponseSender::handle_set(httpd_req_t *req, const std::string &payload)
{
    // Implement your SET logic here
    nlohmann::json response_json = "SET response";
    esp_err_t ret = send_response(req, httpd_req_to_sockfd(req), /*id*/ 0, response_json);
    return ret;
}

esp_err_t ResponseSender::send_response(httpd_req_t *req, int client_fd, int id, const nlohmann::json &data)
{
    // Erstelle ein JSON-Objekt für die Antwort
    nlohmann::json response;
    response["id"] = id;
    response["status"] = "success";
    response["data"] = data;

    // Serialisiere das JSON-Objekt zu einem String
    std::string response_str = response.dump();
    AsyncRespArg *resp_arg = new AsyncRespArg{req->handle, client_fd, strdup(response_str.c_str())};

    esp_err_t ret = httpd_queue_work(req->handle, ws_async_send, resp_arg);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "httpd_queue_work failed with %d", ret);
        delete resp_arg;
    }

    return ret;
}

esp_err_t ResponseSender::send_error_response(httpd_req_t *req, int client_fd, int id, const char *error_message)
{
    // Erstelle ein JSON-Objekt für die Fehlermeldung
    nlohmann::json response;
    response["id"] = id;
    response["status"] = "error";
    response["error"]["message"] = error_message;

    // Serialisiere das JSON-Objekt zu einem String
    std::string response_str = response.dump();
    AsyncRespArg *resp_arg = new AsyncRespArg{req->handle, client_fd, strdup(response_str.c_str())};

    esp_err_t ret = httpd_queue_work(req->handle, ws_async_send, resp_arg);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "httpd_queue_work failed with %d", ret);
        delete resp_arg;
    }

    return ret;
}
void ResponseSender::ws_async_send(void *arg)
{
    AsyncRespArg *resp_arg = static_cast<AsyncRespArg *>(arg);
    if (!resp_arg)
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

esp_err_t ResponseSender::trigger_async_send(httpd_handle_t handle, httpd_req_t *req, const std::string &data)
{
    AsyncRespArg *resp_arg = new AsyncRespArg{handle, httpd_req_to_sockfd(req), data};
    esp_err_t ret = httpd_queue_work(handle, ws_async_send, resp_arg);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "httpd_queue_work failed with %d", ret);
        delete resp_arg;
    }
    return ret;
}
