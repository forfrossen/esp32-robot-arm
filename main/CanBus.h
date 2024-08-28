#ifndef CANBUS_H_old
#define CANBUS_H_old

#include <../components/mcp2515/include/mcp2515.h>
#include <cstdint>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "utils.hpp";

#define CAN_CS_PIN 10
#define CAN_INT_PIN 2
#define CAN_SPEED CAN_500KBPS
#define CLOCK_SPEED MCP_8MHZ
#define MAX_CALLBACKS 10

// Define SPI pins
#define PIN_NUM_MISO GPIO_NUM_19
#define PIN_NUM_MOSI GPIO_NUM_23
#define PIN_NUM_CLK GPIO_NUM_18
#define PIN_NUM_CS GPIO_NUM_5
#define PIN_NUM_INT GPIO_NUM_4

class CanBus
{
private:
  MCP2515 *mcp2515;
  spi_device_handle_t spi;
  QueueHandle_t commandQueue;

  void printCANStatus()
  {
    static const char *TAG = FUNCTION_NAME;
    uint8_t errorFlag = mcp2515->getErrorFlags();

    ESP_LOGE(TAG, "Error Flag: 0x%02X\n", errorFlag);

    if (errorFlag & MCP2515::EFLG_RX0OVR)
      ESP_LOGE(TAG, "Receive Buffer 0 Overflow");
    if (errorFlag & MCP2515::EFLG_TXEP)
      ESP_LOGE(TAG, "Transmit Error Passive");
    if (errorFlag & MCP2515::EFLG_TXBO)
      ESP_LOGE(TAG, "Transmit Bus-Off");
    if (errorFlag & MCP2515::EFLG_RXEP)
      ESP_LOGE(TAG, "Receive Error Passive");
    if (errorFlag & MCP2515::EFLG_TXWAR)
      ESP_LOGE(TAG, "Transmit Warning");
    if (errorFlag & MCP2515::EFLG_RXWAR)
      ESP_LOGE(TAG, "Receive Warning");
    if (errorFlag & MCP2515::EFLG_EWARN)
      ESP_LOGE(TAG, "Error Warning");
  }

public:
  CanBus()
  {
    commandQueue = xQueueCreate(10, sizeof(Command));
    if (commandQueue == NULL)
    {
      ESP_LOGE(FUNCTION_NAME, "Failed to create command queue");
    }
  };

  ~CanBus()
  {
    if (commandQueue != NULL)
    {
      vQueueDelete(commandQueue);
    }
    delete mcp2515;
  }

  void begin()
  {
    static const char *TAG = FUNCTION_NAME;
    esp_err_t ret;
    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };

    // Initialize the SPI bus
    ret = spi_bus_initialize(HSPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);

    spi_device_interface_config_t devcfg = {
        .mode = 0,                         // SPI mode 0
        .clock_speed_hz = 1 * 1000 * 1000, // 1 MHz
        .spics_io_num = PIN_NUM_CS,        // CS pin
        .queue_size = 7,                   // Queue size
        .pre_cb = NULL,
        .post_cb = NULL,
    };

    // Attach the MCP2515 to the SPI bus
    ret = spi_bus_add_device(HSPI_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);

    mcp2515 = new MCP2515(&spi);

    ESP_LOGI(TAG, "BEGIN");

    mcp2515->reset();
    mcp2515->setConfigMode();

    if (mcp2515->setBitrate(CAN_SPEED, CLOCK_SPEED) == MCP2515::ERROR_OK)
    {
      mcp2515->setNormalMode();

      ESP_LOGI(TAG, "CAN BUS Shield init ok!\n");
    }
    else
    {

      ESP_LOGE(TAG, "CAN BUS Shield could not be initialized!");
      while (true)
      {
        vTaskDelay(100 / portTICK_PERIOD_MS);
      }
    }
  }

  uint8_t calculateCRC(uint32_t id, const uint8_t *data, uint8_t length)
  {
    uint8_t crc = id;
    for (uint8_t i = 0; i < length; i++)
    {
      crc += data[i];
    }
    return crc & 0xFF;
  }

  bool sendCANMessage(uint32_t id, uint8_t length, uint8_t *data)
  {
    static const char *TAG = FUNCTION_NAME;
    if (length >= 7)
    {
      length = 7; // Limit length to 7 to make space for CRC
    }

    uint8_t crc = calculateCRC(id, data, length); // Calculate CRC
    data[length] = crc;                           // Append CRC to the end of data

    struct can_frame frame;
    frame.can_id = id;
    frame.can_dlc = length + 1; // Include CRC in the length

    memcpy(frame.data, data, frame.can_dlc);

    if (mcp2515->sendMessage(&frame) == MCP2515::ERROR_OK)
    {
      return true;
    }
    else
    {

      ESP_LOGE(TAG, "Error sending message");
      printCANStatus(); // Print status and error flags

      ESP_LOGE(TAG, "Data: ");
      for (int i = 0; i < frame.can_dlc; i++)
      {
        ESP_LOGE(TAG, "%u", frame.data[i]);
      }
      return false;
    }
  }

  bool listenForMessages(struct can_frame *frame)
  {
    if (mcp2515->readMessage(frame) == MCP2515::ERROR_OK)
    {
      return true;
    }
    return false;
  }
};

#endif // CANBUS_H_old_old
