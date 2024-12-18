#include "TWAIController.hpp"
ESP_EVENT_DEFINE_BASE(MOTOR_EVENTS);
ESP_EVENT_DEFINE_BASE(SYSTEM_EVENTS);

// Initialisierung der TWAI-Schnittstelle
esp_err_t TWAIController::init()
{
    if (_isConnected)
    {
        ESP_LOGW(FUNCTION_NAME, "TWAIController already initialized.");
        return ESP_ERR_INVALID_STATE;
    }
    twai_general_config_t general_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_17, GPIO_NUM_16, TWAI_MODE_NORMAL);
    twai_timing_config_t timing_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t filter_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    uint32_t alerts_to_enable = TWAI_ALERT_ALL;

    general_config.controller_id = 0;
    esp_err_t ret = twai_driver_install(&general_config, &timing_config, &filter_config);
    if (ret != ESP_OK)
    {
        ESP_LOGE(FUNCTION_NAME, "TWAI driver install failed.");
        return ret;
    }
    ESP_LOGD(FUNCTION_NAME, "Driver installed");

    ret = twai_reconfigure_alerts(alerts_to_enable, NULL);
    if (ret != ESP_OK)
    {
        ESP_LOGE(FUNCTION_NAME, "Alerts reconfiguration failed.");
        twai_driver_uninstall();
        return ret;
    }
    ESP_LOGD(FUNCTION_NAME, "Alerts reconfigured");

    ret = twai_start();
    if (ret != ESP_OK)
    {
        ESP_LOGE(FUNCTION_NAME, "TWAI start failed.");
        twai_driver_uninstall();
        return ret;
    }
    ESP_LOGD(FUNCTION_NAME, "Driver started");

    xTaskCreatePinnedToCore(vTask_ERROR, "ErrorDetectionTask", 1024 * 3, this, 1, &taskHandleError, 1);
    assert(taskHandleError != NULL); // Ensure the task was created successfully

    xTaskCreatePinnedToCore(vTask_Reception, "InQueuesTask", 1024 * 3, this, 3, &vTask_Reception_handle, 0);
    assert(vTask_Reception_handle != NULL); // Ensure the task was created successfully

    ret = esp_event_handler_register_with(system_event_loop, SYSTEM_EVENTS, OUTGOING_MESSAGE_EVENT, &outgoing_message_event_handler, this);
    ESP_LOGD(FUNCTION_NAME, "Outgoing message event handler registered");

    _isConnected = true;
    return ESP_OK;
}

TWAIController::~TWAIController()
{
    _isConnected = false;
    twai_stop();
}

bool TWAIController::isConnected()
{
    return _isConnected;
}

esp_err_t TWAIController::setupQueues()
{
    // ESP_LOGD(FUNCTION_NAME, "Creating Queue Buffers");
    // outQ = xQueueCreate(10, sizeof(twai_message_t));

    // assert(outQ != NULL); // Ensure the queue was created successfully

    // xTaskCreatePinnedToCore(vTask_Transmission, "OutQueueTask", 3000, this, 3, &taskHandleTransmission, 0);
    // assert(taskHandleTransmission != NULL); // Ensure the task was created successfully

    // xTaskCreatePinnedToCore(vTask_ERROR, "ErrorDetectionTask", 3000, this, 1, &taskHandleError, 1);
    // assert(taskHandleError != NULL); // Ensure the task was created successfully

    // configASSERT(outQ);

    return ESP_OK;
}

esp_err_t TWAIController::post_event(uint32_t id, twai_message_t *msg)
{
    ESP_LOGD(FUNCTION_NAME, "Posting twai_message to motor event loop");

    CHECK_THAT(msg != nullptr);
    CHECK_THAT(get_event_loop_for_id(id) != nullptr);

    esp_err_t ret = esp_event_post_to(
        get_event_loop_for_id(id),
        MOTOR_EVENTS,
        INCOMING_MESSAGE_EVENT,
        (void *)msg,
        sizeof(twai_message_t),
        portMAX_DELAY);

    CHECK_THAT(ret == ESP_OK);

    return ESP_OK;
}

esp_err_t TWAIController::register_motor_id(uint32_t id, esp_event_loop_handle_t motor_event_loop)
{
    motor_event_loops.emplace(id, motor_event_loop);
    return ESP_OK;
}

esp_event_loop_handle_t TWAIController::get_event_loop_for_id(uint32_t id)
{
    return motor_event_loops[id];
}

TWAIController::ERROR TWAIController::disconnectCan()
{
    _isConnected = false;
    return ERROR_OK;
}

void TWAIController::vTask_ERROR(void *pvParameters)
{
    TWAIController *twai_controller = static_cast<TWAIController *>(pvParameters);
    while (1)
    {
        uint32_t alerts;
        twai_read_alerts(&alerts, portMAX_DELAY); // Block indefinitely until an alert occurs

        if (alerts & TWAI_ALERT_TX_IDLE || alerts & TWAI_ALERT_TX_SUCCESS || alerts & TWAI_ALERT_RX_DATA)
        {
            continue;
        }

        twai_controller->handleAlerts(alerts);
    }
}

void TWAIController::vTask_Reception(void *pvParameters)
{
    TWAIController *twai_controller = static_cast<TWAIController *>(pvParameters);

    while (1)
    {
        twai_message_t msg;
        esp_err_t ret = twai_receive(&msg, portMAX_DELAY);
        CONT_IF_CHECK_FAILS(ret == ESP_OK);
        if (ret != ESP_OK)
        {
            ESP_LOGE(FUNCTION_NAME, "Failed to receive message");
            continue;
        }
        // #log_twai_message(&msg, true);
        ESP_LOGD(FUNCTION_NAME, "Received message");
        xSemaphoreTake(twai_controller->twai_mutex, portMAX_DELAY);
        assert(ret == ESP_OK);
        twai_controller->post_event(msg.identifier, &msg);
        ESP_LOGD(FUNCTION_NAME, "\t==> Received message enqueued successfully!");
        xSemaphoreGive(twai_controller->twai_mutex);
    }
}

void TWAIController::outgoing_message_event_handler(void *args, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    TWAIController *instance = static_cast<TWAIController *>(args);
    ESP_LOGD(FUNCTION_NAME, "new outgoing message event");
    if (event_base != SYSTEM_EVENTS || event_id != OUTGOING_MESSAGE_EVENT)
    {
        ESP_LOGE(FUNCTION_NAME, "Invalid event");
        return;
    }

    if (xSemaphoreTake(instance->twai_mutex, portMAX_DELAY) != pdTRUE)
    {
        ESP_LOGE(FUNCTION_NAME, "Failed to take mutex");
        return;
    }

    twai_message_t *msg = (twai_message_t *)event_data;

    log_twai_message(msg);

    ESP_LOGD(FUNCTION_NAME, "Trying to send message");

    esp_err_t result = twai_transmit(msg, portMAX_DELAY);
    xSemaphoreGive(instance->twai_mutex);

    if (result != ESP_OK)
    {
        ESP_LOGE(FUNCTION_NAME, "\t==> Failed to send message");
        instance->handleTransmitError(&result);
    }
    ESP_LOGD(FUNCTION_NAME, "\t==> Successfully sent message");
}

// void TWAIController::vTask_Transmission(void *pvParameters)
// {
//     TWAIController *twai_controller = static_cast<TWAIController *>(pvParameters);

//     while (1)
//     {
//         twai_message_t outQmsg;
//         if (xQueueReceive(twai_controller->outQ, &outQmsg, portMAX_DELAY) == pdPASS)
//         {
//             xSemaphoreTake(twai_controller->twai_mutex, portMAX_DELAY);
//             // outQmsg.data[outQmsg.data_length_code] = 32;
//             // twai_controller->calculate_crc(&outQmsg);

//             ESP_LOGD(FUNCTION_NAME, "Sending message");

//             auto result = twai_transmit(&outQmsg, portMAX_DELAY);

//             if (result != ESP_OK)
//             {
//                 ESP_LOGE(FUNCTION_NAME, "\t==> Failed to send message");
//                 twai_controller->handleTransmitError(&result);
//                 continue;
//             }
//             ESP_LOGD(FUNCTION_NAME, "\t==> Successfully sent message");
//             xSemaphoreGive(twai_controller->twai_mutex);
//             vTaskDelay(200 / portTICK_PERIOD_MS);
//         }
//     }
// }

void TWAIController::handleTransmitError(esp_err_t *error)
{
    xEventGroupClearBits(system_event_group, TWAI_READY);
    xEventGroupSetBits(system_event_group, TWAI_ERROR);

    switch (*error)
    {
    case ESP_ERR_INVALID_ARG:
        ESP_LOGE(FUNCTION_NAME, "Arguments are invalid");
        break;
    case ESP_ERR_TIMEOUT:
        ESP_LOGE(FUNCTION_NAME, "Timed out waiting for space on TX queue");
        break;
    case ESP_FAIL:
        ESP_LOGE(FUNCTION_NAME, "TX queue is disabled and another message is currently transmitting");
        break;
    case ESP_ERR_INVALID_STATE:
        ESP_LOGE(FUNCTION_NAME, "TWAI driver is not in running state, or is not installed");
        break;
    case ESP_ERR_NOT_SUPPORTED:
        ESP_LOGE(FUNCTION_NAME, "Listen Only Mode does not support transmissions");
        break;
    default:
        ESP_LOGE(FUNCTION_NAME, "Unknown error occurred");
        break;
    }

    assert(*error != ESP_ERR_INVALID_STATE);
}

void TWAIController::handleAlerts(uint32_t alerts)
{

    ESP_LOGE(FUNCTION_NAME, "Alerts triggered: %lu", alerts);
    if (alerts & TWAI_ALERT_TX_IDLE)
    {
        ESP_LOGE(FUNCTION_NAME, "No more messages to transmit");
    }
    if (alerts & TWAI_ALERT_TX_SUCCESS)
    {
        ESP_LOGE(FUNCTION_NAME, "The previous transmission was successful");
    }
    if (alerts & TWAI_ALERT_RX_DATA)
    {
        ESP_LOGE(FUNCTION_NAME, "A frame has been received and added to the RX queue");
    }
    if (alerts & TWAI_ALERT_BELOW_ERR_WARN)
    {
        ESP_LOGE(FUNCTION_NAME, "Both error counters have dropped below error warning limit");
    }
    if (alerts & TWAI_ALERT_ERR_ACTIVE)
    {
        ESP_LOGE(FUNCTION_NAME, "TWAI controller has become error active");
    }
    if (alerts & TWAI_ALERT_RECOVERY_IN_PROGRESS)
    {
        ESP_LOGE(FUNCTION_NAME, "TWAI controller is undergoing bus recovery");
    }
    if (alerts & TWAI_ALERT_BUS_RECOVERED)
    {
        ESP_LOGE(FUNCTION_NAME, "TWAI controller has successfully completed bus recovery");
    }
    if (alerts & TWAI_ALERT_ARB_LOST)
    {
        ESP_LOGE(FUNCTION_NAME, "The previous transmission lost arbitration");
    }
    if (alerts & TWAI_ALERT_ABOVE_ERR_WARN)
    {
        ESP_LOGE(FUNCTION_NAME, "One of the error counters have exceeded the error warning limit");
    }
    if (alerts & TWAI_ALERT_BUS_ERROR)
    {
        ESP_LOGE(FUNCTION_NAME, "A (Bit, Stuff, CRC, Form, ACK) error has occurred on the bus");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    if (alerts & TWAI_ALERT_TX_FAILED)
    {
        ESP_LOGE(FUNCTION_NAME, "The previous transmission has failed (for single shot transmission)");
    }
    if (alerts & TWAI_ALERT_RX_QUEUE_FULL)
    {
        ESP_LOGE(FUNCTION_NAME, "The RX queue is full causing a frame to be lost");
    }
    if (alerts & TWAI_ALERT_ERR_PASS)
    {
        ESP_LOGE(FUNCTION_NAME, "TWAI controller has become error passive");
    }
    if (alerts & TWAI_ALERT_BUS_OFF)
    {
        ESP_LOGE(FUNCTION_NAME, "Bus-off condition occurred. TWAI controller can no longer influence bus");
    }
    if (alerts & TWAI_ALERT_RX_FIFO_OVERRUN)
    {
        ESP_LOGE(FUNCTION_NAME, "An RX FIFO overrun has occurred");
    }
    if (alerts & TWAI_ALERT_TX_RETRIED)
    {
        ESP_LOGE(FUNCTION_NAME, "A message transmission was cancelled and retried due to an errata workaround");
    }
    if (alerts & TWAI_ALERT_PERIPH_RESET)
    {
        ESP_LOGE(FUNCTION_NAME, "The TWAI controller was reset");
    }
}
