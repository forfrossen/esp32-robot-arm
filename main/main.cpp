#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/semphr.h"
#include "esp_err.h"
#include "CommandMapper.hpp"
#include "CanBus.hpp"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_app_format.h"
#include <iostream>
#include <regex>
#include <string>
#include "utils.hpp"
#include "CANServo.hpp"

CanBus *canBus;
CommandMapper *commandMapper = new CommandMapper();

std::map<uint8_t, CANServo *> Servos;

const char compile_date[] = __DATE__ " " __TIME__;
extern "C" void app_main()
{
  esp_log_level_set("*", ESP_LOG_INFO);
  /*

    const std::map<std::string, esp_log_level_t> logLevels = {
        {"CanBus::sendCANMessage", ESP_LOG_ERROR},
        {"CanBus::vTask_handleReceiveQueue", ESP_LOG_INFO},
        {"CANServo::vTask_queryPosition", ESP_LOG_ERROR},
        {"CANServo::sendCommand", ESP_LOG_INFO},
        {"CANServo::handleReceivedMessage", ESP_LOG_INFO},
        {"CANServo::decodeMessage", ESP_LOG_INFO},
        {"CANServo::handleQueryMotorPositionResponse", ESP_LOG_INFO},
        {"QueryMotorPositionCommand::execute", ESP_LOG_ERROR},
        {"CommandMapper::getCommandNameFromCode", ESP_LOG_ERROR},
    };
    // Apply log levels for specific tags
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

  canBus = new CanBus();

  canBus->begin();
  vTaskDelay(pdMS_TO_TICKS(100));
  canBus->connectCan();
  vTaskDelay(pdMS_TO_TICKS(100));
  canBus->setupInterrupt();
  vTaskDelay(pdMS_TO_TICKS(100));
  canBus->setupQueues();
  vTaskDelay(pdMS_TO_TICKS(100));
  // Allow other core to finish initialization

  // Initialisierung der Servo42D_CAN Instanzen
  // Servos[0x01] = new CANServo(0x01, canBus, commandMapper);
  // Servos[0x02] = new CANServo(0x02, canBus, commandMapper);
  Servos[0x03] = new CANServo(0x03, canBus, commandMapper);
}
