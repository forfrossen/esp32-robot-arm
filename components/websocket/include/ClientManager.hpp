#ifndef CLIENT_MANAGER_H
#define CLIENT_MANAGER_H

#include "TypeDefs.hpp"
#include "utils.hpp"
#include <esp_err.h>
#include <esp_http_server.h>
#include <esp_log.h>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <nvs_flash.h>
#include <string>
typedef struct
{
    httpd_handle_t handle;
    httpd_req_t *req;
} client_details_t;

class ClientManager
{
public:
    ClientManager();
    ~ClientManager();

    esp_err_t get_client(std::string client_id, client_details_t &client_details);
    esp_err_t upsert_client(std::string client_id, httpd_handle_t handle, httpd_req_t *req);
    esp_err_t remove_client(std::string client_id);
    esp_err_t load_clients();
    esp_err_t save_clients();

private:
    std::map<std::string, client_details_t> clients;
    esp_err_t load_from_nvs();
    esp_err_t save_to_nvs();
};
#endif // CLIENT_MANAGER_H