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
#include "CanBus.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_app_format.h"
#include "CANServo.hpp"
#include <iostream>
#include <regex>
#include <string>
#include "utils.hpp"

#include "Commands/Command.hpp"
#include "Commands/Query/QueryMotorStatusCommand.hpp"
#include "Commands/Query/QueryMotorPositionCommand.hpp"

CanBus *canBus = new CanBus();
CommandMapper *commandMapper;
std::map<uint8_t, CANServo *> Servos;

const char compile_date[] = __DATE__ " " __TIME__;

void checkForMessages()
{
  static const char *TAG = FUNCTION_NAME;

  can_frame msg;
  while (canBus->listenForMessages(&msg))
  {

    uint32_t id = msg.can_id;

    if (Servos[id] == nullptr)
    {
      ESP_LOGI(TAG, "Servo not found for ID: %lu", id);
      continue;
    }

    Servos[id]->handleReceivedMessage(&msg);
  }
}

void task_checkMessages(void *pvParameters)
{
  // UBaseType_t uxHighWaterMark;
  static const char *TAG = FUNCTION_NAME;

  // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);

  for (;;)
  {
    // ESP_LOGI(TAG, "New iteration of taskCheckCanMessages");
    // ESP_LOGI(TAG, "High water mark before calling checkForMessages: %d", uxHighWaterMark);

    checkForMessages();

    // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    // ESP_LOGI(TAG, "High water mark before calling checkForMessages: %d", uxHighWaterMark);

    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void task_QueryPosition(void *pvParameters)
{
  static const char *TAG = FUNCTION_NAME;
  vTaskDelay(2000 / portTICK_PERIOD_MS);

  for (;;)
  {
    ESP_LOGI(TAG, "New iteration of taskQueryMotorPosition");
    for (auto &pair : Servos)
    {
      Command *queryMotorPosition = new QueryMotorPositionCommand(pair.second);
      queryMotorPosition->execute();
      delete queryMotorPosition;
      vTaskDelay(200 / portTICK_PERIOD_MS);
    }
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

extern "C" void app_main()
{
  static const char *TAG = FUNCTION_NAME;

  // static const char *TAG =
  esp_log_level_set("*", ESP_LOG_INFO);
  esp_log_level_set("void CommandMapper::getCommandNameFromCode", ESP_LOG_ERROR);
  // esp_log_level_set("wifi", ESP_LOG_WARN);
  // esp_log_level_set("dhcpc", ESP_LOG_INFO);

  ESP_LOGI(TAG, "Hello world! Robot Arm starting up...");

  // Allow other core to finish initialization
  vTaskDelay(pdMS_TO_TICKS(100));

  /** Event handler for Ethernet events */
  // static void eth_event_handler(void *arg, esp_event_base_t event_base,
  //                             int32_t event_id, void *event_data)

  // xTaskCreate(task_randomLogger, "LogMsgCreator", 128, NULL, 2, NULL);
  ESP_LOGI(TAG, "Hallo, Test from Arm !!!");
  ESP_LOGI(TAG, "Build date: %s", compile_date);

  canBus->begin();

  commandMapper = new CommandMapper();

  // Initialisierung der Servo42D_CAN Instanzen
  // Servos[0x01] = new CANServo(0x01, canBus, commandMapper);
  // Servos[0x02] = new CANServo(0x02, canBus, commandMapper);
  Servos[0x03] = new CANServo(0x03, canBus, commandMapper);

  xTaskCreatePinnedToCore(task_checkMessages, "taskCheckMessages", 4096, NULL, 1, NULL, 1);
  // xTaskCreatePinnedToCore(task_QueryPosition, "taskQueryMotorPosition", 512 * 2 * 5, NULL, 2, NULL, 0);
}
