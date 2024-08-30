#ifndef CANBUS_H
#define CANBUS_H

#include <cstdint>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "utils.hpp"
#include <map>
#include <driver/twai.h>

#define MAX_CALLBACKS 10
// #DEFINE bitrate  500Kbs;
// #DEFINE CAN_CLOCK canClock = 8MHZ;

class CanBus
{
private:
  std::map<uint32_t, QueueHandle_t> inQs;
  TaskHandle_t taskHandleReception;
  TaskHandle_t taskHandleTransmission;
  TaskHandle_t taskHandleError;

  bool interrupt = false;
  bool _isConnected = false;
  bool _timestampEnabled = false;
  bool _listenOnly = false;
  bool _loopback = false;
  static const char CR = '\r';
  static const char BEL = 7;
  static const uint16_t TIMESTAMP_LIMIT = 0xEA60;

public:
  enum ERROR
  {
    ERROR_OK,
    ERROR_CONNECTED,
    ERROR_NOT_CONNECTED,
    ERROR_UNKNOWN_COMMAND,
    ERROR_INVALID_COMMAND,
    ERROR_ERROR_twai_message_t_NOT_SUPPORTED,
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

  ERROR connectCan();
  ERROR disconnectCan();
  ERROR setupQueues();

  void registerInQueue(uint32_t canId, QueueHandle_t inQ);

  bool isConnected();

  static void vTask_Transmission(void *pvParameters);
  static void vTask_Reception(void *pvParameters);
  static void vTask_ERROR(void *pvParameters);

  QueueHandle_t outQ;

  void calculateCRC(twai_message_t *msg);
  bool transmit(twai_message_t msg);
  void handleTransmitError(esp_err_t *error);
  void handleAlerts(uint32_t alerts);
};

#endif // CANBUS_H
