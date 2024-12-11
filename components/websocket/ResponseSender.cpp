#include "ResponseSender.hpp"

static const char *TAG = "ResponseSender";

ResponseSender::ResponseSender(
    httpd_handle_t server,
    std::shared_ptr<ClientManager> client_manager)
    : server(server), client_manager(client_manager) {}

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

esp_err_t ResponseSender::send_error_response(httpd_req_t *req, int client_fd, int id, const char *error_message)
{
    // Erstelle ein JSON-Objekt für die Fehlermeldung
    nlohmann::json response;
    response["id"] = id;
    response["status"] = "error";
    response["error"]["message"] = error_message;

    // Serialisiere das JSON-Objekt zu einem String
    std::string response_str = response.dump();
    AsyncRespArg *resp_arg = new AsyncRespArg{req->handle, httpd_req_to_sockfd(req), response_str.c_str()};

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

esp_err_t ResponseSender::send_rpc_response(rpc_event_data *data)
{
    auto command = static_cast<IWsCommand *>(data->command);
    auto req = command->get_req();
    ESP_RETURN_ON_FALSE(command != nullptr, ESP_ERR_INVALID_ARG, TAG, "Command is null");

    auto client_id = command->get_client_id();
    client_details_t client_details;
    ESP_RETURN_ON_ERROR(client_manager->get_client(client_id, client_details), TAG, "Failed to get client");

    nlohmann::json response;

    response["id"] = command->get_id();
    response["status"] = command->get_result();

    if (command->get_result())
    {
        response["data"] = command->get_result();
    }
    else
    {
        response["error"]["message"] = "An Error Occurred";
    }

    ESP_LOGD(TAG, "Sending response to client %s", client_id.c_str());
    // return send_response(client_details.req, req., command->get_id(), response);
    return trigger_async_send(req->handle, req, response.dump());
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

    AsyncRespArg *resp_arg = new AsyncRespArg{
        req->handle,
        httpd_req_to_sockfd(req),
        response.dump()};

    esp_err_t ret = httpd_queue_work(req->handle, ws_async_send, resp_arg);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "httpd_queue_work failed with %d", ret);
        delete resp_arg;
    }

    return ret;
}
