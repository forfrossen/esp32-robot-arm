

#include "Events.hpp"

#include "Wifi.hpp"

#include "RobotArm.hpp"
#include "WebSocketServer.hpp"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "sdkconfig.h"
#include "utils.hpp"
#include <inttypes.h>
#include <iostream>
#include <memory>
#include <regex>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

ESP_EVENT_DEFINE_BASE(SYSTEM_EVENTS);
ESP_EVENT_DEFINE_BASE(MOTOR_EVENTS);

const char compile_date[] = __DATE__ " " __TIME__;

RobotArm *robot_arm;
static httpd_handle_t server = NULL;

extern "C" void app_main()
{

    esp_err_t ret;
    esp_log_level_set("*", ESP_LOG_INFO);
    /*

      const std::map<std::string, esp_log_level_t> logLevels = {
          {"TWAIController::transmit", ESP_LOG_ERROR},
          {"TWAIController::vTask_Reception", ESP_LOG_INFO},
          {"MotorController::vTask_queryPosition", ESP_LOG_ERROR},
          {"MotorController::sendCommand", ESP_LOG_INFO},
          {"MotorController::handle_received_message", ESP_LOG_INFO},
          {"MotorController::decodeMessage", ESP_LOG_INFO},
          {"MotorController::handleQueryMotorPositionResponse", ESP_LOG_INFO},
          {"QueryMotorPositionCommand::execute", ESP_LOG_ERROR},
      };
      // Apply log levels for specific FUNCTION_NAMEs
      for (const auto &x : logLevels)
      {
        esp_log_level_set(x.first.c_str(), x.second);
        ESP_LOGI("app_main", "Set log level for %s to %d", x.first.c_str(), x.second);
      }
    */
    ESP_LOGI(FUNCTION_NAME, "Hello world! Robot Arm starting up...");

    ESP_LOGI(FUNCTION_NAME, "Hallo, Test from Arm !!!");
    ESP_LOGI(FUNCTION_NAME, "Build date: %s", compile_date);

    ESP_ERROR_CHECK(esp_netif_init());

    robot_arm = new RobotArm();

    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(FUNCTION_NAME, "ESP_WIFI_MODE_STA");

    Wifi::wifi_init_sta();

    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));

    server = start_webserver();
}
