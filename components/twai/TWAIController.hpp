#ifndef TWAICONTROLLER_H
#define TWAICONTROLLER_H

#include "../common/utils.hpp"
#include "TypeDefs.hpp"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include <cstdint>
#include <cstring>
#include <driver/twai.h>
#include <map>

class TWAIController
{
private:
    SemaphoreHandle_t twai_mutex = NULL;
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

    TWAIController(esp_event_loop_handle_t system_event_loop) : system_event_loop(system_event_loop)
    {
        esp_err_t ret;
        ESP_LOGI(FUNCTION_NAME, "TWAIController constructor called");
        twai_mutex = xSemaphoreCreateMutex();
        if (!_isConnected)
        {
            ret = init();
            if (ret != ESP_OK)
            {
                ESP_LOGE(FUNCTION_NAME, "TWAIController initialization failed.");
            }
        }
        ESP_LOGI(FUNCTION_NAME, "TWAIController initialized.");

        // ret = setupQueues();
        // if (ret != ESP_OK)
        // {
        //     ESP_LOGE(FUNCTION_NAME, "TWAIController setupQueues failed.");
        // }
        // ESP_LOGI(FUNCTION_NAME, "TWAIController setupQueues successful.");
    }

    ~TWAIController();

    ERROR connectCan();
    ERROR disconnectCan();
    esp_err_t setupQueues();

    esp_err_t register_motor_id(uint32_t canId, esp_event_loop_handle_t motor_event_loop);

    bool isConnected();

    static void vTask_Transmission(void *pvParameters);
    static void vTask_Reception(void *pvParameters);
    static void vTask_ERROR(void *pvParameters);

    std::map<uint32_t, esp_event_loop_handle_t> motor_event_loops;
    esp_event_loop_handle_t get_event_loop_for_id(uint32_t);
    esp_event_loop_handle_t system_event_loop;
    TaskHandle_t vTask_Reception_handle;

    bool
    transmit(twai_message_t msg);
    void handleTransmitError(esp_err_t *error);
    void handleAlerts(uint32_t alerts);

    void post_event(uint32_t id, twai_message_t *msg);

    static void outgoing_message_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data);
};

#endif // CANBUS_H
