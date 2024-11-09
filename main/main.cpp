#define MAGIC_ENUM_RANGE_MIN 0x00
#define MAGIC_ENUM_RANGE_MAX 0xFF

#include "esp_http_server.h"
#include "esp_netif.h"
#include "sdkconfig.h"
#include <esp_err.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>

#include <inttypes.h>
#include <iostream>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "Events.hpp"

#include "RobotArm.hpp"
#include "utils.hpp"
#include <nvs_flash.h>

#include "WebSocket.hpp"
#include "WebSocketServer.hpp"
#include "Wifi.hpp"

ESP_EVENT_DEFINE_BASE(SYSTEM_EVENTS);

const char compile_date[] = __DATE__ " " __TIME__;

esp_event_loop_args_t system_loop_args = {
    .queue_size = 10,
    .task_name = "system_event_loop_task",
    .task_priority = 1,
    .task_stack_size = 1024 * 10,
    .task_core_id = tskNO_AFFINITY};

esp_event_loop_handle_t system_event_loop;
EventGroupHandle_t system_event_group;

RobotArm *robot_arm;
static httpd_handle_t server = NULL;
WebSocket *ws;
static const char *TAG = "MAIN";

extern "C" void app_main()
{
    esp_err_t ret;
    esp_log_level_set("*", ESP_LOG_DEBUG);

    // WebSocket and WebServer logs
    esp_log_level_set("httpd_parse", ESP_LOG_INFO);
    esp_log_level_set("httpd_txrx", ESP_LOG_INFO);
    esp_log_level_set("httpd_ws", ESP_LOG_INFO);
    esp_log_level_set("httpd_uri", ESP_LOG_INFO);
    esp_log_level_set("httpd_sess", ESP_LOG_INFO);
    esp_log_level_set("httpd", ESP_LOG_INFO);

    // Wifi logs
    esp_log_level_set("wifi_init_default", ESP_LOG_INFO);
    esp_log_level_set("esp_netif_lwip", ESP_LOG_INFO);
    esp_log_level_set("esp_netif_handlers", ESP_LOG_WARN);
    esp_log_level_set("nvs", ESP_LOG_INFO);
    esp_log_level_set("wifi", ESP_LOG_INFO);
    esp_log_level_set("wifi:wifi", ESP_LOG_WARN);
    esp_log_level_set("Wifi::wifi_init_sta", ESP_LOG_INFO);
    esp_log_level_set("esp_netif_objects", ESP_LOG_INFO);
    esp_log_level_set("wifi:mode", ESP_LOG_INFO);
    esp_log_level_set("esp_netif_objects", ESP_LOG_INFO);
    esp_log_level_set("wifi:state", ESP_LOG_INFO);
    esp_log_level_set("wifi_init", ESP_LOG_INFO);

    // General logs
    esp_log_level_set("efuse", ESP_LOG_INFO);
    esp_log_level_set("cpu_start", ESP_LOG_INFO);
    esp_log_level_set("esp_event", ESP_LOG_INFO);
    esp_log_level_set("esp_event_loop", ESP_LOG_INFO);

    // const std::map<std::string, esp_log_level_t> logLevels = {
    //     {"TWAIController::transmit", ESP_LOG_ERROR},
    //     {"TWAIController::vTask_Reception", ESP_LOG_INFO},
    //     {"MotorController::vTask_queryPosition", ESP_LOG_ERROR},
    //     {"MotorController::sendCommand", ESP_LOG_INFO},
    //     {"MotorController::handle_received_message", ESP_LOG_INFO},
    //     {"MotorController::decodeMessage", ESP_LOG_INFO},
    //     {"MotorController::handleQueryMotorPositionResponse", ESP_LOG_INFO},
    //     {"QueryMotorPositionCommand::execute", ESP_LOG_ERROR},
    // };
    // // Apply log levels for specific FUNCTION_NAMEs
    // for (const auto &x : logLevels)
    // {
    //     esp_log_level_set(x.first.c_str(), x.second);
    //     ESP_LOGD("app_main", "Set log level for %s to %d", x.first.c_str(), x.second);
    // }

    ESP_LOGD(TAG, "Hello world! Robot Arm starting up...");

    ESP_LOGD(TAG, "Hallo, Test from Arm !!!");
    ESP_LOGD(TAG, "Build date: %s", compile_date);

    ESP_ERROR_CHECK(esp_netif_init());

    // esp_event_loop_handle_t tmp_handle = nullptr;

    // ESP_ERROR_CHECK(esp_event_loop_create(&system_loop_args, &tmp_handle));
    // assert(tmp_handle != nullptr);
    // system_event_loop = std::make_shared<esp_event_loop_handle_t>(tmp_handle);

    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    system_event_group = xEventGroupCreate();
    ESP_RETURN_VOID_ON_FALSE(system_event_group != NULL, TAG, "Failed to create system event group");

    // Initialize the event loop handle

    esp_event_loop_create(&system_loop_args, &system_event_loop);

    ESP_LOGI(TAG, "System event loop created: %p", system_event_loop);
    // Initialize the RobotArm, which will register the event handler
    robot_arm = new RobotArm(system_event_loop, system_event_group);

    ESP_LOGD(FUNCTION_NAME, "ESP_WIFI_MODE_STA");
    Wifi::wifi_init_sta();

    ws = new WebSocket(system_event_loop, system_event_group);

    // Start WebSocket server
    ret = ws->start();
    if (ret != ESP_OK)
    {
        ESP_LOGE("MAIN", "Failed to start WebSocket: %s", esp_err_to_name(ret));
    }
}
