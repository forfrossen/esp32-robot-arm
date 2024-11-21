#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include "ClientManager.hpp"
#include "Events.hpp"
#include "ResponseSender.hpp"
#include "SetRunLevelCommand.h"
#include "TypeDefs.hpp"
#include "Utilities.hpp"
#include "WsCommandDefs.hpp"
#include "WsCommandFactory.hpp"
#include "magic_enum.hpp"
#include <cassert>
#include <cstring>
#include <esp_check.h>
#include <esp_err.h>
#include <esp_event.h>
#include <esp_http_server.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <memory>
#include <nlohmann/json.hpp>

class EventManager
{
public:
    EventManager(
        esp_event_loop_handle_t system_event_loop,
        EventGroupHandle_t &system_event_group,
        std::shared_ptr<WsCommandFactory> command_factory,
        std::shared_ptr<ResponseSender> response_sender,
        std::shared_ptr<ClientManager> client_manager);

    ~EventManager();

    esp_err_t on_rpc_request(httpd_req_t *req, ws_message_t msg);

    esp_err_t on_set_motor_command(httpd_req_t *req, ws_message_t msg);

    esp_err_t on_system_command(httpd_req_t *req, ws_message_t msg);
    esp_err_t send_system_command(httpd_req_t *req, ws_message_t msg);

    esp_err_t register_handlers();
    esp_err_t unregister_handlers();
    esp_err_t post_event(system_event_id_t event, remote_control_event_t message);
    esp_err_t set_runlevel(httpd_req_t *req, ws_message_t msg);

    esp_err_t handle_command(httpd_req_t *req, ws_message_t msg, esp_err_t (EventManager::*command_func)(httpd_req_t *, ws_message_t));

private:
    esp_event_loop_handle_t system_event_loop;
    EventGroupHandle_t &system_event_group;
    std::shared_ptr<WsCommandFactory> command_factory;
    std::shared_ptr<ResponseSender> response_sender;
    std::shared_ptr<ClientManager> client_manager;

    static void connect_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void disconnect_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void http_server_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void property_change_event_handler(void *args, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void on_rpc_response(void *args, esp_event_base_t event_base, int32_t event_id, void *event_data);
};

#endif // EVENT_MANAGER_H
