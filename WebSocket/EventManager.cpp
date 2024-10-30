#include "EventManager.hpp"
#include "esp_log.h"
#include "Utilities.hpp"
#include <cassert>

static const char *TAG = "EventManager";

EventManager::EventManager(esp_event_loop_handle_t system_event_loop, EventGroupHandle_t &system_event_group)
    : system_event_loop(system_event_loop),
      system_event_group(system_event_group) {}

EventManager::~EventManager() {
    unregister_handlers();
}

esp_err_t EventManager::register_handlers() {
    // Register connect and disconnect handlers
    ESP_RETURN_ON_ERROR(
        esp_event_handler_register(
            WIFI_EVENT,
            WIFI_EVENT_STA_DISCONNECTED,
            &EventManager::disconnect_handler,
            this),
        TAG,
        "Failed to register disconnect handler");

    ESP_RETURN_ON_ERROR(
        esp_event_handler_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &EventManager::connect_handler,
            this),
        TAG,
        "Failed to register connect handler");

    // Register property change event handler
    ESP_RETURN_ON_ERROR(
        esp_event_handler_register_with(
            system_event_loop,
            SYSTEM_EVENTS,
            PROPERTY_CHANGE_EVENT,
            &EventManager::property_change_event_handler,
            this),
        TAG,
        "Failed to register property change event handler");

    return ESP_OK;
}

esp_err_t EventManager::unregister_handlers() {
    esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &EventManager::disconnect_handler);
    esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &EventManager::connect_handler);
    esp_event_handler_unregister_with(system_event_loop, SYSTEM_EVENTS, PROPERTY_CHANGE_EVENT, &EventManager::property_change_event_handler);
    return ESP_OK;
}

esp_err_t EventManager::post_event(system_event_id_t event, remote_control_event_t message) {
    ESP_LOGD(TAG, "Posting system event %d with message: %d", static_cast<int>(event), static_cast<int>(message));
    assert(system_event_loop != nullptr);

    esp_err_t ret = esp_event_post_to(
        system_event_loop,
        SYSTEM_EVENTS,
        event,
        static_cast<const void *>(&message),
        sizeof(remote_control_event_t),
        portMAX_DELAY);

    ESP_RETURN_ON_ERROR(ret, TAG, "Error posting system event");
    return ESP_OK;
}

esp_err_t EventManager::set_runlevel(int runlevel, httpd_req_t *req) {
    switch (runlevel) {
        case 0:
            xEventGroupSetBits(system_event_group, RUNLEVEL_0);
            xEventGroupClearBits(system_event_group, RUNLEVEL_1 | RUNLEVEL_2 | RUNLEVEL_3);
            break;
        case 1:
            xEventGroupSetBits(system_event_group, RUNLEVEL_1 | RUNLEVEL_0);
            xEventGroupClearBits(system_event_group, RUNLEVEL_2 | RUNLEVEL_3);
            break;
        case 2:
            xEventGroupSetBits(system_event_group, RUNLEVEL_2 | RUNLEVEL_1 | RUNLEVEL_0);
            xEventGroupClearBits(system_event_group, RUNLEVEL_3);
            break;
        case 3:
            xEventGroupSetBits(system_event_group, RUNLEVEL_3 | RUNLEVEL_2 | RUNLEVEL_1 | RUNLEVEL_0);
            break;
        default:
            ESP_LOGW(TAG, "Invalid runlevel: %d", runlevel);
            return ESP_ERR_INVALID_ARG;
    }

    // Optionally, send a success message back to the client
    // This may require further refactoring depending on your architecture

    return ESP_OK;
}

