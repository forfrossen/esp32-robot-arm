#define MAGIC_ENUM_RANGE_MIN 0x00
#define MAGIC_ENUM_RANGE_MAX 0xFF

#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "sdkconfig.h"

#include <inttypes.h>
#include <iostream>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "Events.hpp"

#include "RobotArm.hpp"
#include "nvs_flash.h"
#include "utils.hpp"

#include "WebSocketServer.hpp"
#include "Wifi.hpp"

ESP_EVENT_DEFINE_BASE(SYSTEM_EVENTS);

const char compile_date[] = __DATE__ " " __TIME__;

esp_event_loop_args_t system_loop_args = {
    .queue_size = 10,
    .task_name = "system_event_task",
    .task_priority = 1,
    .task_stack_size = 1024 * 10,
    .task_core_id = tskNO_AFFINITY};

esp_event_loop_handle_t system_event_loop;

RobotArm *robot_arm;
static httpd_handle_t server = NULL;
WebSocket *ws;

extern "C" void app_main()
{
    esp_err_t ret;
    esp_log_level_set("*", ESP_LOG_DEBUG);

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

    ESP_LOGD(FUNCTION_NAME, "Hello world! Robot Arm starting up...");

    ESP_LOGD(FUNCTION_NAME, "Hallo, Test from Arm !!!");
    ESP_LOGD(FUNCTION_NAME, "Build date: %s", compile_date);

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create(&system_loop_args, &system_event_loop));

    robot_arm = new RobotArm(system_event_loop);

    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGD(FUNCTION_NAME, "ESP_WIFI_MODE_STA");

    Wifi::wifi_init_sta();
    ws = new WebSocket(server, system_event_loop);
}
