#include "CommandMapper.hpp"
#include "MotorController.hpp"
#include "RobotArm.hpp"
#include "TWAIController.hpp"
#include "Wifi.hpp"

#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "utils.hpp"
#include <inttypes.h>
#include <iostream>
#include <regex>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
;
TWAIController *twai_controller;
CommandMapper *command_mapper = new CommandMapper();
RobotArm *robot_arm;

const char compile_date[] = __DATE__ " " __TIME__;
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
          {"MotorController::handleReceivedMessage", ESP_LOG_INFO},
          {"MotorController::decodeMessage", ESP_LOG_INFO},
          {"MotorController::handleQueryMotorPositionResponse", ESP_LOG_INFO},
          {"QueryMotorPositionCommand::execute", ESP_LOG_ERROR},
          {"CommandMapper::getCommandNameFromCode", ESP_LOG_ERROR},
      };
      // Apply log levels for specific FUNCTION_NAMEs
      for (const auto &x : logLevels)
      {
        esp_log_level_set(x.first.c_str(), x.second);
        ESP_LOGI("app_main", "Set log level for %s to %d", x.first.c_str(), x.second);
      }
    */
    ESP_LOGI(FUNCTION_NAME, "Hello world! Robot Arm starting up...");

    // Allow other core to finish initialization
    vTaskDelay(pdMS_TO_TICKS(100));

    /** Event handler for Ethernet events */
    // static void eth_event_handler(void *arg, esp_event_base_t event_base,
    //                             int32_t event_id, void *event_data)

    // xTaskCreate(task_randomLogger, "LogMsgCreator", 128, NULL, 2, NULL);
    ESP_LOGI(FUNCTION_NAME, "Hallo, Test from Arm !!!");
    ESP_LOGI(FUNCTION_NAME, "Build date: %s", compile_date);

    twai_controller = new TWAIController();
    vTaskDelay(pdMS_TO_TICKS(100));

    ret = twai_controller->setupQueues();
    if (ret != ESP_OK)
    {
        ESP_LOGE(FUNCTION_NAME, "Error setting up queues");
    }
    vTaskDelay(pdMS_TO_TICKS(100));

    // Allow other core to finish initialization

    // Initialisierung der Servo42D_CAN Instanzen
    // Servos[0x01] = new MotorController(0x01, twai_controller, command_mapper);
    // Servos[2] = new MotorController(0x02, twai_controller, command_mapper);
    // Servos[0x03] = new MotorController(0x03, twai_controller, command_mapper);
    robot_arm = new RobotArm(twai_controller, command_mapper);

    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(FUNCTION_NAME, "ESP_WIFI_MODE_STA");

    Wifi::wifi_init_sta();
}
