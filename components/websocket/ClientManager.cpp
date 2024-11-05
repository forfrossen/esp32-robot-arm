#include "ClientManager.hpp"
using json = nlohmann::json;

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

    for (auto const &[key, val] : clients)
    {
        ESP_LOGD(TAG, "Client in registry: %s", key.c_str());
        CONT_IF_CHECK_FAILS(key == client_id);
        ESP_LOGI(TAG, "Client found: %s", client_id.c_str());
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

esp_err_t ClientManager::save_to_nvs()
{
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return err;
    }

    while (clients.size() > 10)
    {
        auto oldest_client = clients.begin();
        clients.erase(oldest_client);
    }

    json j;
    for (const auto &client : clients)
    {
        j[client.first] = {
            {"handle", reinterpret_cast<uintptr_t>(client.second.handle)},
            {"req", reinterpret_cast<uintptr_t>(client.second.req)}};
    }

    std::string json_str = j.dump();
    size_t required_size = json_str.size() + 1;

    err = nvs_set_str(nvs_handle, "clients", json_str.c_str());
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) setting clients in NVS!", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }

    err = nvs_commit(nvs_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) committing clients to NVS!", esp_err_to_name(err));
    }

    nvs_close(nvs_handle);
    return err;
}

esp_err_t ClientManager::load_from_nvs()
{
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return err;
    }

    size_t required_size;
    err = nvs_get_str(nvs_handle, "clients", NULL, &required_size);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) getting clients size from NVS!", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }

    char *json_str = (char *)malloc(required_size);
    err = nvs_get_str(nvs_handle, "clients", json_str, &required_size);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) getting clients from NVS!", esp_err_to_name(err));
        free(json_str);
        nvs_close(nvs_handle);
        return err;
    }

    json j = json::parse(json_str);
    for (auto it = j.begin(); it != j.end(); ++it)
    {
        std::string client_id = it.key();
        client_details_t client_details;
        client_details.handle = reinterpret_cast<httpd_handle_t>(it.value()["handle"].get<uintptr_t>());
        client_details.req = reinterpret_cast<httpd_req_t *>(it.value()["req"].get<uintptr_t>());
        clients[client_id] = client_details;
    }

    free(json_str);
    nvs_close(nvs_handle);
    return ESP_OK;
}