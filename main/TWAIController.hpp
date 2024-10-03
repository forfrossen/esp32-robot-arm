#ifndef TWAICONTROLLER_H
#define TWAICONTROLLER_H

#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "utils.hpp"
#include <cstdint>
#include <cstring>
#include <driver/twai.h>
#include <map>

class TWAIController
{
private:
    SemaphoreHandle_t twai_mutex = NULL;
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
    esp_err_t init();

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

    TWAIController();
    ~TWAIController();

    ERROR connectCan();
    ERROR disconnectCan();
    esp_err_t setupQueues();

    esp_err_t register_motor_id(uint32_t canId);

    bool isConnected();

    static void vTask_Transmission(void *pvParameters);
    static void vTask_Reception(void *pvParameters);
    static void vTask_ERROR(void *pvParameters);

    QueueHandle_t outQ;

    std::map<uint32_t, QueueHandle_t> inQs;
    QueueHandle_t get_inQ_for_id(uint32_t);

    bool transmit(twai_message_t msg);
    void handleTransmitError(esp_err_t *error);
    void handleAlerts(uint32_t alerts);
};

#endif // CANBUS_H
