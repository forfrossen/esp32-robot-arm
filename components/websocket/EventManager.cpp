#include "EventManager.hpp"

static const char *TAG = "EventManager";

EventManager::EventManager(esp_event_loop_handle_t system_event_loop, EventGroupHandle_t &system_event_group, std::shared_ptr<WsCommandFactory> command_factory)
    : system_event_loop(system_event_loop),
      system_event_group(system_event_group),
      command_factory(command_factory)
{
    assert(system_event_group != nullptr);
    assert(system_event_loop != nullptr);
    assert(command_factory != nullptr);
}

EventManager::~EventManager()
{
    unregister_handlers();
}

esp_err_t EventManager::register_handlers()
{
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

    // Register HTTP server event handler
    ESP_RETURN_ON_ERROR(
        esp_event_handler_register(
            ESP_HTTP_SERVER_EVENT,
            ESP_EVENT_ANY_ID,
            &EventManager::http_server_event_handler,
            this),
        TAG,
        "Failed to register HTTP server event handler");

    return ESP_OK;
}

esp_err_t EventManager::unregister_handlers()
{
    esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &EventManager::disconnect_handler);
    esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &EventManager::connect_handler);
    esp_event_handler_unregister_with(system_event_loop, SYSTEM_EVENTS, PROPERTY_CHANGE_EVENT, &EventManager::property_change_event_handler);
    esp_event_handler_unregister(ESP_HTTP_SERVER_EVENT, ESP_EVENT_ANY_ID, &EventManager::property_change_event_handler);

    return ESP_OK;
}

esp_err_t EventManager::post_event(system_event_id_t event, remote_control_event_t message)
{
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

esp_err_t EventManager::set_runlevel(ws_payload_t payload)
{
    RunMode run_mode;
    CHECK_THAT(get_run_mode(payload, run_mode) == ESP_OK);
    CHECK_THAT(run_mode != RunMode::UNKNOWN);
    CHECK_THAT(command_factory != nullptr);
    ESP_LOGD(TAG, "Creating command, to set RunMode to: %s", magic_enum::enum_name(run_mode).data());
    std::variant<int, std::string, RunMode, ws_set_target_position_payload_t> payload = run_mode;
    ws_payload_t payload2 = run_mode;

    CHECK_THAT(command_factory->create(ws_command_id::SET_RUNMODE, payload) == ESP_OK);

    return ESP_OK;
}

void EventManager::connect_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    EventManager *manager = static_cast<EventManager *>(arg);
    if (manager)
    {
        ESP_LOGD(TAG, "Connect event received");
        // Handle connect event, e.g., start WebSocket server
        // This might require access to WebSocketServer instance
    }
    else
    {
        ESP_LOGE(TAG, "EventManager instance is null in connect_handler");
    }
}

void EventManager::disconnect_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    EventManager *manager = static_cast<EventManager *>(arg);
    if (manager)
    {
        ESP_LOGD(TAG, "Disconnect event received");
        // Handle disconnect event, e.g., stop WebSocket server
        // This might require access to WebSocketServer instance
    }
    else
    {
        ESP_LOGE(TAG, "EventManager instance is null in disconnect_handler");
    }
}

void EventManager::http_server_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    EventManager *manager = static_cast<EventManager *>(arg);
    RETURN_VOID_IF(manager == nullptr);
    ESP_LOGI(TAG, "HTTP server event received: %ld - %s", event_id, magic_enum::enum_name(static_cast<esp_http_server_event_id_t>(event_id)).data());
}

void EventManager::property_change_event_handler(void *args, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    EventManager *manager = static_cast<EventManager *>(args);
    if (manager)
    {
        ESP_LOGD(TAG, "Property change event received");
        // Handle property change event, e.g., notify clients
        // This might require access to ResponseSender or WebSocketServer
    }
    else
    {
        ESP_LOGE(TAG, "EventManager instance is null in property_change_event_handler");
    }
}