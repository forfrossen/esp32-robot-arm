#include "WebSocket.hpp"

static const char *TAG = "WebSocket";

WebSocket::WebSocket(
    esp_event_loop_handle_t system_event_loop,
    EventGroupHandle_t &system_event_group)
    : system_event_loop(system_event_loop),
      system_event_group(system_event_group)
{
    ESP_LOGD(TAG, "WebSocket instance created");
    assert(system_event_group != nullptr);
    assert(system_event_loop != nullptr);

    command_config_map = {
        {ws_command_id::SET_RUNMODE, {system_event_loop, SYSTEM_EVENTS, SET_RUN_MODE_EVENT}},
        {ws_command_id::START_MOTORS, {system_event_loop, SYSTEM_EVENTS, REMOTE_CONTROL_EVENT}},
        {ws_command_id::STOP_MOTORS, {system_event_loop, SYSTEM_EVENTS, REMOTE_CONTROL_EVENT}}};

    // Initialize submodules
    command_factory = std::make_shared<WsCommandFactory>(command_config_map);
    assert(command_factory != nullptr);
    response_sender = std::make_shared<ResponseSender>(nullptr);
    assert(response_sender != nullptr);
    event_manager = std::make_shared<EventManager>(system_event_loop, system_event_group, command_factory);
    assert(event_manager != nullptr);
    request_handler = std::make_shared<RequestHandler>(response_sender, event_manager);
    assert(request_handler != nullptr);

    server = std::make_shared<WebSocketServer>(request_handler, response_sender, event_manager);
}

WebSocket::~WebSocket()
{
    ESP_LOGW(TAG, "WebSocket instance destroyed");
    stop();
}

esp_err_t WebSocket::start()
{
    ESP_LOGI(TAG, "Starting WebSocket");
    ESP_RETURN_ON_ERROR(event_manager->register_handlers(),
                        TAG,
                        "Failed to register event handlers");

    ESP_RETURN_ON_ERROR(server->start(),
                        TAG,
                        "Failed to start WebSocket server");

    xEventGroupSetBits(system_event_group, WEBSOCKET_READY);
    return ESP_OK;
}

esp_err_t WebSocket::stop()
{
    ESP_LOGD(TAG, "Stopping WebSocket");
    ESP_RETURN_ON_ERROR(
        server->stop(),
        TAG,
        "Failed to stop WebSocket server");

    event_manager->unregister_handlers();
    xEventGroupClearBits(system_event_group, WEBSOCKET_READY);

    return ESP_OK;
}