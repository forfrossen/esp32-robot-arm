#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

extern "C" void app_main()
{

  static const char *TAG = "app_main";
  printf("Hello world!\n");
  ESP_LOGI(TAG, "Hello world!");
  while (1)
    ;
}