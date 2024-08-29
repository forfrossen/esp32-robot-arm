#ifndef CANBUS_H
#define CANBUS_H

#include <../components/mcp2515/include/mcp2515.h>
#include <cstdint>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "utils.hpp"
#include <map>

#define CAN_CS_PIN 10
#define CAN_INT_PIN 2

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
  CAN_SPEED bitrate = CAN_500KBPS;
  CAN_CLOCK canClock = MCP_8MHZ;
  spi_device_handle_t spi;

  std::map<uint32_t, QueueHandle_t> inQs;
  TaskHandle_t taskHandleCheckMessages;

  bool interrupt = false;
  bool _isConnected = false;
  bool _timestampEnabled = false;
  bool _listenOnly = false;
  bool _loopback = false;
  static const char CR = '\r';
  static const char BEL = 7;
  static const uint16_t TIMESTAMP_LIMIT = 0xEA60;

  void printCANStatus();

public:
  enum ERROR
  {
    ERROR_OK,
    ERROR_CONNECTED,
    ERROR_NOT_CONNECTED,
    ERROR_UNKNOWN_COMMAND,
    ERROR_INVALID_COMMAND,
    ERROR_ERROR_FRAME_NOT_SUPPORTED,
    ERROR_OUTQUEUE_NOT_INITIALIZED,
    ERROR_INQUEUE_NOT_INITIALIZED,
    ERROR_ENQUEUE_FAILED,
    ERROR_BUFFER_OVERFLOW,
    ERROR_SERIAL_TX_OVERRUN,
    ERROR_LISTEN_ONLY,
    ERROR_MCP2515_INIT,
    ERROR_MCP2515_INIT_CONFIG,
    ERROR_MCP2515_INIT_BITRATE,
    ERROR_MCP2515_INIT_SET_MODE,
    ERROR_MCP2515_SEND,
    ERROR_MCP2515_READ,
    ERROR_MCP2515_FILTER,
    ERROR_MCP2515_ERRIF,
    ERROR_MCP2515_MERRF,
  };

  CanBus();
  ~CanBus();

  ERROR setupSpi();
  ERROR connectCan();
  ERROR disconnectCan();
  ERROR setupInterrupt();
  ERROR setupQueues();

  ERROR parseTransmit(const char *buffer, int length, struct can_frame *frame);
  ERROR createTransmit(const struct can_frame *frame, char *buffer, const int length);

  ERROR writeStream(const char character);
  ERROR writeStream(const char *buffer);

  ERROR receiveCan(const MCP2515::RXBn rxBuffer);
  ERROR receiveCanFrame(const struct can_frame *frame);

  void registerInQueue(canid_t canId, QueueHandle_t inQ);

  bool isConnected();
  void begin();

  static void irqHandler(void *arg);
  static void vTask_handleSendQueue(void *pvParameters);
  static void vTask_handleReceiveQueue(void *pvParameters);

  QueueHandle_t outQ;

  uint8_t calculateCRC(uint32_t id, const uint8_t *data, uint8_t length);
  bool sendCANMessage(can_frame frame);
  bool listenForMessages(struct can_frame *frame);

  uint16_t getTimestamp();
  MCP2515 *getMcp2515();
};

#endif // CANBUS_H
