#include "CanBus.hpp"

CanBus::CanBus()
{
    twai_general_config_t generalConfig = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_17, GPIO_NUM_16, TWAI_MODE_NORMAL);
    twai_timing_config_t timingConfig = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t filterConfig = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    uint32_t alerts_to_enable = TWAI_ALERT_ALL;

    ESP_ERROR_CHECK(twai_driver_install(&generalConfig, &timingConfig, &filterConfig));
    ESP_LOGI(FUNCTION_NAME, "Driver installed");

    ESP_ERROR_CHECK(twai_reconfigure_alerts(alerts_to_enable, NULL));
    ESP_LOGI(FUNCTION_NAME, "Alerts reconfigured");

    ESP_ERROR_CHECK(twai_start());
    ESP_LOGI(FUNCTION_NAME, "Driver started");

    _isConnected = true;

    ESP_ERROR_CHECK(setupQueues());
}

CanBus::~CanBus()
{
    _isConnected = false;
    twai_stop();
}

bool CanBus::isConnected()
{
    return _isConnected;
}

esp_err_t CanBus::setupQueues()
{
    ESP_LOGI(FUNCTION_NAME, "Creating Queue Buffers");
    outQ = xQueueCreate(10, sizeof(twai_message_t));

    assert(outQ != NULL); // Ensure the queue was created successfully

    xTaskCreatePinnedToCore(vTask_Transmission, "OutQueueTask", 3000, this, 3, &taskHandleTransmission, 0);
    assert(taskHandleTransmission != NULL); // Ensure the task was created successfully

    xTaskCreatePinnedToCore(vTask_Reception, "InQueuesTask", 3000, this, 3, &taskHandleReception, 0);
    assert(taskHandleReception != NULL); // Ensure the task was created successfully

    xTaskCreatePinnedToCore(vTask_ERROR, "ErrorDetectionTask", 3000, this, 1, &taskHandleError, 1);
    assert(taskHandleError != NULL); // Ensure the task was created successfully

    configASSERT(outQ);

    return ESP_OK;
}

esp_err_t CanBus::registerInQueue(uint32_t id, QueueHandle_t inQ)
{
    inQs.emplace(id, inQ);
    return ESP_OK;
}

CanBus::ERROR CanBus::disconnectCan()
{
    _isConnected = false;
    return ERROR_OK;
}

void CanBus::vTask_ERROR(void *pvParameters)
{
    CanBus *canBus = static_cast<CanBus *>(pvParameters);
    while (1)
    {
        uint32_t alerts;
        twai_read_alerts(&alerts, portMAX_DELAY); // Block indefinitely until an alert occurs

        if (alerts & TWAI_ALERT_TX_IDLE || alerts & TWAI_ALERT_TX_SUCCESS || alerts & TWAI_ALERT_RX_DATA)
        {
            continue;
        }

        canBus->handleAlerts(alerts);
    }
}

void CanBus::vTask_Reception(void *pvParameters)
{
    CanBus *canBus = static_cast<CanBus *>(pvParameters);

    while (1)
    {
        twai_message_t msg;
        esp_err_t ret = twai_receive(&msg, portMAX_DELAY);
        assert(ret == ESP_OK); // Ensure the message was received successfully
        ESP_LOGI(FUNCTION_NAME, "Message received");

        assert(canBus->inQs.count(msg.identifier) > 0);

        ESP_LOGI(FUNCTION_NAME, "Queue found for identifier %lu", msg.identifier);
        BaseType_t xStatus = xQueueSendToBack(canBus->inQs[msg.identifier], &msg, 0);
        assert(xStatus == pdPASS);

        ESP_LOGI(FUNCTION_NAME, "\t==> Received message enqueued successfully!");

        // ESP_LOGE(FUNCTION_NAME, "\t==> Error enqueuing received message!");
        //  continue;
    }
}

void CanBus::calculateCRC(twai_message_t *msg)
{
    uint8_t crc = msg->identifier;

    for (uint8_t i = 0; i < msg->data_length_code - 1; i++)
    {
        crc += msg->data[i];
    }

    msg->data[msg->data_length_code - 1] = crc;
}

void CanBus::vTask_Transmission(void *pvParameters)
{
    CanBus *canBus = static_cast<CanBus *>(pvParameters);

    while (1)
    {
        twai_message_t outQmsg;
        if (xQueueReceive(canBus->outQ, &outQmsg, portMAX_DELAY) == pdPASS)
        {
            // outQmsg.data[outQmsg.data_length_code] = 32;
            canBus->calculateCRC(&outQmsg);

            ESP_LOGI(FUNCTION_NAME, "Sending message");

            auto result = twai_transmit(&outQmsg, portMAX_DELAY);

            if (result != ESP_OK)
            {
                ESP_LOGE(FUNCTION_NAME, "\t==> Failed to send message");
                canBus->handleTransmitError(&result);
                continue;
            }
            ESP_LOGI(FUNCTION_NAME, "\t==> Successfully sent message");
        }
    }
}

void CanBus::handleTransmitError(esp_err_t *error)
{
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

void CanBus::handleAlerts(uint32_t alerts)
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
