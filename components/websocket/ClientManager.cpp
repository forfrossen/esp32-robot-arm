#include "ClientManager.hpp"

static const char *TAG = "ClientManager";

ClientManager::ClientManager() {}

ClientManager::~ClientManager() {}

esp_err_t ClientManager::get_client(std::string client_id, client_details_t &client_details)
{
    auto it = clients.find(client_id);
    if (it != clients.end())
    {
        ESP_LOGI(TAG, "Client found: %s", client_id.c_str());
        client_details = it->second;
        return ESP_OK;
    }
    ESP_LOGE(TAG, "Client not found: %s", client_id.c_str());
    return ESP_ERR_NOT_FOUND;
}

esp_err_t ClientManager::upsert_client(std::string client_id, httpd_handle_t handle, httpd_req_t *req)
{
    clients[client_id] = {handle, req};
    return ESP_OK;
}

esp_err_t ClientManager::remove_client(std::string client_id)
{
    clients.erase(client_id);
    return ESP_OK;
}