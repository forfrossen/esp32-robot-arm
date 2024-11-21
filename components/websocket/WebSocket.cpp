#include "WebSocket.hpp"
ESP_EVENT_DEFINE_BASE(RPC_EVENTS);

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
        {system_command_id_t::SET_RUNLEVEL, {system_event_loop, SYSTEM_EVENTS, SET_RUN_MODE_EVENT}},
        {system_command_id_t::START_MOTORS, {system_event_loop, SYSTEM_EVENTS, START_MOTORS}},
        {system_command_id_t::STOP_MOTORS, {system_event_loop, SYSTEM_EVENTS, STOP_MOTORS}},
        {system_command_id_t::MOTOR_CONTROL_COMMAND, {system_event_loop, SYSTEM_EVENTS, REMOTE_CONTROL_EVENT}},
    };

    // Initialize submodules
    client_manager = std::make_shared<ClientManager>();
    assert(client_manager != nullptr);

    command_factory = std::make_shared<WsCommandFactory>(command_config_map, system_event_loop, RPC_EVENTS, SEND_RESPONSE);
    assert(command_factory != nullptr);

    response_sender = std::make_shared<ResponseSender>(nullptr, client_manager);
    assert(response_sender != nullptr);

    event_manager = std::make_shared<EventManager>(system_event_loop, system_event_group, command_factory, response_sender, client_manager);
    assert(event_manager != nullptr);

    request_handler = std::make_shared<RequestHandler>(response_sender, event_manager, client_manager);
    assert(request_handler != nullptr);

    server = std::make_shared<WebSocketServer>(request_handler, response_sender, event_manager, client_manager);
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
                        TAG, "Failed to register event handlers");

    ESP_RETURN_ON_ERROR(server->start(),
                        TAG, "Failed to start WebSocket server");

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
