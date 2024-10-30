#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include "esp_event.h"
#include "esp_err.h"
#include <memory>

enum class remote_control_event_t {
    START_MOTORS,
    STOP_MOTORS,
    // Add other events as needed
};

class EventManager {
public:
    EventManager(esp_event_loop_handle_t system_event_loop, EventGroupHandle_t &system_event_group);
    ~EventManager();

    esp_err_t register_handlers();
    esp_err_t unregister_handlers();
    esp_err_t post_event(system_event_id_t event, remote_control_event_t message);
    esp_err_t set_runlevel(int runlevel, httpd_req_t *req);

private:
    esp_event_loop_handle_t system_event_loop;
    EventGroupHandle_t &system_event_group;
