#include "MotorController.hpp"

#define STEPS_PER_REVOLUTION 16384

void MotorController::vTask_queryPosition(void *pvParameters)
{

    MotorController *instance = static_cast<MotorController *>(pvParameters);
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    for (;;)
    {
        if (instance->is_healthy() == ESP_OK)
        {
            ESP_LOGI(FUNCTION_NAME, "New iteration of taskQueryMotorPosition");
            instance->query_position();
        }

        vTaskDelay(4000 / portTICK_PERIOD_MS);
    }
}

void MotorController::vTask_queryStatus(void *pvParameters)
{
    MotorController *instance = static_cast<MotorController *>(pvParameters);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    for (;;)
    {
        ESP_LOGI(FUNCTION_NAME, "New iteration of taskQueryStatus");
        instance->query_status();

        vTaskDelay(4000 / portTICK_PERIOD_MS);
    }
}

/*
void MotorController::vTask_handleInQ(void *vParameters)
{
    MotorController *instance = static_cast<MotorController *>(vParameters);
    for (;;)
    {
        twai_message_t twai_message_t;
        xQueueReceive(instance->inQ, &twai_message_t, portMAX_DELAY);
        ESP_LOGI(FUNCTION_NAME, "Received message from inQ with ID: %lu", twai_message_t.identifier);
        instance->handle_received_message(&twai_message_t);
    }
}
*/

void MotorController::vtask_sendPositon(void *pvParameters)
{
    MotorController *instance = static_cast<MotorController *>(pvParameters);
    vTaskDelay(8000 / portTICK_PERIOD_MS);
    for (;;)
    {
        vTaskDelay(6000 / portTICK_PERIOD_MS);

        if (instance->is_healthy() != ESP_OK)
        {
            continue;
        }

        ESP_LOGI(FUNCTION_NAME, "New iteration of taskSendRandomTargetPositionCommands");
        instance->set_target_position();
    }
}

MotorController::MotorController(
    std::shared_ptr<MotorControllerDependencies> dependencies) : canId(dependencies->id),
                                                                 command_factory(dependencies->command_factory),
                                                                 inQ(dependencies->inQ),
                                                                 outQ(dependencies->outQ),
                                                                 command_mapper(dependencies->command_mapper),
                                                                 command_lifecycle_registry(dependencies->command_lifecycle_registry)
{
    ESP_LOGI(FUNCTION_NAME, "New Servo42D_CAN object created with CAN ID: %lu", canId);

    BaseType_t xReturned;
    if (inQ == nullptr)
    {
        ESP_LOGE(FUNCTION_NAME, "inQ is nullptr");
    }
    if (outQ == nullptr)
    {
        ESP_LOGE(FUNCTION_NAME, "outQ is nullptr");
    }

    command_factory->query_motor_status_command()->build_and_send();

    xReturned = xTaskCreatePinnedToCore(&MotorController::vTask_queryPosition, "TASK_queryPosition", 1024 * 3, this, 2, NULL, 1);
    if (xReturned != pdPASS)
    {
        ESP_LOGE(FUNCTION_NAME, "Failed to create task_queryPosition");
    }

    xReturned = xTaskCreatePinnedToCore(&MotorController::vtask_sendPositon, "TASK_SendRandomTargetPositionCommands", 1024 * 3, this, 4, NULL, 1);
    if (xReturned != pdPASS)
    {
        ESP_LOGE(FUNCTION_NAME, "Failed to create task_sendRandomTargetPositionCommands");
    }
    // xTaskCreatePinnedToCore(&MotorController::vTask_handleInQ, "TASK_handleInQ", 1024 * 3, this, 2, NULL, 0);

    configASSERT(vTask_queryPosition);
    // configASSERT(vTask_handleInQ);
    configASSERT(vtask_sendPositon);
}

esp_err_t MotorController::query_position()
{
    command_factory->query_motor_position_command()->build_and_send();
    return ESP_OK;
}

esp_err_t MotorController::is_healthy()
{
    command_factory->query_motor_status_command()->build_and_send();
    if (!is_connected)
    {
        ESP_LOGE(FUNCTION_NAME, "Motor is not connected.");
        return ESP_FAIL;
    }
    if (error_counter > 5)
    {
        ESP_LOGE(FUNCTION_NAME, "Error counter exceeded 5.");
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t MotorController::set_target_position()
{
    int position = (esp_random() % 10) * STEPS_PER_REVOLUTION; // Random value between 0 and 99
    int speed = esp_random() % 1600;
    int acceleration = esp_random() % 255;
    bool absolute = esp_random() % 2;

    speed = 250;
    acceleration = 255;
    absolute = false;

    set_state(MotorControllerFSM::State::REQUESTED);
    esp_err_t ret = command_factory->create_set_target_position_command()->set_position(position).set_speed(speed).set_acceleration(acceleration).set_absolute(absolute).build_and_send();

    return ret;
}

esp_err_t MotorController::query_status()
{
    esp_err_t ret = command_factory->query_motor_status_command()->build_and_send();
    return ret;
}

void MotorController::set_state(MotorControllerFSM::State newState)
{
    state_machine.set_state(newState);
}

void MotorController::handle_received_message(twai_message_t *msg)
{

    if (!msg->data[0])
    {
        ESP_LOGE(FUNCTION_NAME, "Error: code is empty!");
        return;
    }

    char command_name[50];
    command_mapper->get_command_name_from_code(msg->data[0], command_name);

    ESP_LOGI(FUNCTION_NAME, "ID: %lu\t length: %u\tcode: %d\tcommandName: %s", canId, msg->data_length_code, msg->data[0], command_name);

    uint8_t *data = msg->data;
    uint8_t length = msg->data_length_code;

    switch (data[0])
    {
    case 0x30:
        handleQueryMotorPositionResponse(msg);
        break;
        break;
    case 0xF5:
    case 0xF4:
        handleSetPositionResponse(msg);
        break;
    case 0xF1:
        handle_query_status_response(msg);
        break;
    case 0x90:
        handeSetHomeResponse(msg);
        break;
    default:
        ESP_LOGE(FUNCTION_NAME, "unimplemented code: %02X", data[0]);
        ESP_LOGI(FUNCTION_NAME, "Raw code byte: %u", data[0]);
        for (int i = 0; i < length; i++)
        {
            ESP_LOGI(FUNCTION_NAME, "Data[%d]: %d", i, data[i]);
        }
    }
}

void MotorController::handle_query_status_response(twai_message_t *msg)
{
    uint8_t status = msg->data[1];
    is_connected = true;
    switch (status)
    {
    case 0:
        ESP_LOGE(FUNCTION_NAME, "Abfrage fehlgeschlagen");
        is_connected = false;
        motor_moving_state = MotorMovingState::UNKNOWN;
        break;
    case 1:
        ESP_LOGI(FUNCTION_NAME, "Motor gestoppt");
        motor_moving_state = MotorMovingState::STOPPED;
        break;
    case 2:
        ESP_LOGI(FUNCTION_NAME, "Motor beschleunigt");
        motor_moving_state = MotorMovingState::ACCELERATING;
        break;
    case 3:
        ESP_LOGI(FUNCTION_NAME, "Motor verlangsamt");
        motor_moving_state = MotorMovingState::DECELERATING;
        break;
    case 4:
        ESP_LOGI(FUNCTION_NAME, "Motor volle Geschwindigkeit");
        motor_moving_state = MotorMovingState::FULL_SPEED;
        break;
    case 5:
        ESP_LOGI(FUNCTION_NAME, "Motor fährt nach Hause");
        motor_moving_state = MotorMovingState::HOMING;
        break;
    case 6:
        ESP_LOGI(FUNCTION_NAME, "Motor wird kalibriert");
        motor_moving_state = MotorMovingState::CALIBRATING;
        break;
    default:
        ESP_LOGI(FUNCTION_NAME, "Unbekannter Status");
        is_connected = false;
        motor_moving_state = MotorMovingState::UNKNOWN;
        break;
    }
}

void MotorController::handleQueryMotorPositionResponse(twai_message_t *msg)
{

    if (msg->data_length_code != 8 || msg->data[0] != 0x30)
    {
        ESP_LOGE(FUNCTION_NAME, "Invalid response length or code.");
        return;
    }

    carry_value = (msg->data[1] << 24) | (msg->data[2] << 16) | (msg->data[3] << 8) | msg->data[4];
    encoder_value = (msg->data[5] << 8) | msg->data[6];

    ESP_LOGI(FUNCTION_NAME, "Carry value: %lu", carry_value);
    ESP_LOGI(FUNCTION_NAME, "Encoder value: %u", encoder_value);
}

void MotorController::handleSetPositionResponse(twai_message_t *msg)
{

    if (msg->data_length_code != 3)
    {
        ESP_LOGE(FUNCTION_NAME, "Invalid response length.");
        return;
    }
    if (msg->data[0] != 0xF5 && msg->data[0] != 0xF4)
    {
        ESP_LOGE(FUNCTION_NAME, "Unexpected command code: %u", msg->data[0]);
        return;
    }

    uint8_t status = msg->data[1];

    std::string F5Status;
    switch (status)
    {
    case 0:
        F5Status = "Run failed";
        state_machine.set_state(MotorControllerFSM::State::ERROR);
        break;
    case 1:
        F5Status = "Run starting";
        state_machine.set_state(MotorControllerFSM::State::MOVING);
        break;
    case 2:
        F5Status = "Run complete";
        state_machine.set_state(MotorControllerFSM::State::COMPLETED);
        break;
    case 3:
        F5Status = "End limit stopped";
        state_machine.set_state(MotorControllerFSM::State::COMPLETED);
        break;
    default:
        F5Status = "Unknown status";
        state_machine.set_state(MotorControllerFSM::State::ERROR);
        break;
    }

    ESP_LOGI(FUNCTION_NAME, "Set Position Response: %s", F5Status.c_str());
}

void MotorController::handeSetHomeResponse(twai_message_t *msg)
{

    if (msg->data_length_code != 3)
    {
        ESP_LOGE(FUNCTION_NAME, "Invalid response length: %u", msg->data_length_code);
        return;
    }

    if (msg->data[0] != 0x90)
    {
        ESP_LOGE(FUNCTION_NAME, "Unexpected command code: %u", msg->data[0]);
        return;
    }

    uint8_t status = msg->data[1];
    uint8_t crc = msg->data[2];

    std::string statusMessage;
    switch (status)
    {
    case 0:
        statusMessage = "Set home failed.";
        break;
    case 1:
        statusMessage = "Set home in progress...";
        break;
    case 2:
        statusMessage = "Set home completed.";
        break;
    default:
        statusMessage = "Unknown status.";
        break;
    }

    ESP_LOGI(FUNCTION_NAME, "Set Home Response: %s", statusMessage.c_str());
    ESP_LOGI(FUNCTION_NAME, "CRC: %u", crc);
}

void MotorController::handleSetWorkModeResponse(twai_message_t *msg)
{

    if (msg->data[1] == 1)
    {
        ESP_LOGI(FUNCTION_NAME, "Set Work Mode: Success");
    }
    else
    {
        ESP_LOGE(FUNCTION_NAME, "Set Work Mode: Failed");
    }
}

void MotorController::handleSetCurrentResponse(twai_message_t *msg)
{

    if (msg->data[1] == 1)
    {
        ESP_LOGI(FUNCTION_NAME, "Set Current: Success");
    }
    else
    {
        ESP_LOGE(FUNCTION_NAME, "Set Current: Failed");
    }
}
