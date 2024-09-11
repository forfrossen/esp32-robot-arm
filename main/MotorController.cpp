#include "MotorController.hpp"
#include "Commands/Command.hpp"
#include "Commands/Query/QueryMotorPositionCommand.hpp"
#include "Commands/SetTargetPositionCommand.hpp"
#include "esp_random.h"
#include "utils.hpp"

void MotorController::vTask_queryPosition(void *pvParameters)
{

    MotorController *instance = static_cast<MotorController *>(pvParameters);
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    for (;;)
    {
        if (instance->errorCounter > 5)
        {
            ESP_LOGE(FUNCTION_NAME, "Error counter exceeded 5. Stopping taskQueryMotorPosition");
            vTaskDelete(NULL);
        }

        ESP_LOGI(FUNCTION_NAME, "New iteration of taskQueryMotorPosition");

        Command *queryMotorPosition = new QueryMotorPositionCommand(instance);
        queryMotorPosition->execute();
        delete queryMotorPosition;

        vTaskDelay(4000 / portTICK_PERIOD_MS);
    }
}

void MotorController::vTask_handleInQ(void *vParameters)
{
    MotorController *instance = static_cast<MotorController *>(vParameters);
    for (;;)
    {
        twai_message_t twai_message_t;
        xQueueReceive(instance->inQ, &twai_message_t, portMAX_DELAY);
        ESP_LOGI(FUNCTION_NAME, "Received message from inQ with ID: %lu", twai_message_t.identifier);
        instance->handleReceivedMessage(&twai_message_t);
    }
}

void MotorController::task_sendPositon(void *pvParameters)
{
    MotorController *instance = static_cast<MotorController *>(pvParameters);
    vTaskDelay(8000 / portTICK_PERIOD_MS);
    for (;;)
    {
        ESP_LOGI(FUNCTION_NAME, "New iteration of taskSendRandomTargetPositionCommands");

        // Generate random values
        int randomValue = esp_random() % 1000; // Random value between 0 and 99
        int randomSpeed = esp_random() % 2000;
        int randomAccel = esp_random() % 255;

        // Command *setTargetPositionCommand = new SetTargetPositionCommand(instance, randomValue * 1000, randomSpeed, randomAccel, true);
        // setTargetPositionCommand->execute();
        // delete setTargetPositionCommand;

        SetTargetPositionCommand setTargetPositionCommand = instance->setTargetPositionCommand(instance, randomValue * 1000, randomSpeed, randomAccel, true);
        uint8_t *data = setTargetPositionCommand.getData();

        instance->sendCommand(data, 7);

        vTaskDelay(4000 / portTICK_PERIOD_MS);
    }
}

MotorController::MotorController(uint32_t id, TWAIController *twai_controller, CommandMapper *command_mapper) : canId(id), twai_controller(twai_controller), command_mapper(command_mapper)
{
    ESP_LOGI(FUNCTION_NAME, "New Servo42D_CAN object created with CAN ID: %lu", canId);

    inQ = xQueueCreate(10, sizeof(twai_message_t));
    ESP_ERROR_CHECK(twai_controller->registerInQueue(canId, inQ));

    outQ = twai_controller->outQ;
    configASSERT(inQ);
    configASSERT(outQ);

    // Temporary Tasks
    xTaskCreatePinnedToCore(&MotorController::vTask_queryPosition, "TASK_queryPosition", 1024 * 3, this, 2, NULL, 1);
    xTaskCreatePinnedToCore(&MotorController::task_sendPositon, "TASK_SendRandomTargetPositionCommands", 1024 * 3, this, 4, NULL, 1);

    xTaskCreatePinnedToCore(&MotorController::vTask_handleInQ, "TASK_handleInQ", 1024 * 3, this, 2, NULL, 0);

    configASSERT(vTask_queryPosition);
    configASSERT(vTask_handleInQ);
    configASSERT(task_sendPositon);
}

void MotorController::setState(StateMachine::State newState)
{
    stateMachine.setState(newState);
}

void MotorController::handleReceivedMessage(twai_message_t *twai_message_t)
{

    if (!twai_message_t->data[0])
    {
        ESP_LOGE(FUNCTION_NAME, "Error: code is empty!");
        return;
    }

    char commandName[50];
    command_mapper->getCommandNameFromCode(twai_message_t->data[0], commandName);

    ESP_LOGI(FUNCTION_NAME, "ID: %lu\t length: %u\tcode: %d\tcommandName: %s", canId, twai_message_t->data_length_code, twai_message_t->data[0], commandName);

    uint8_t *data = twai_message_t->data;
    uint8_t length = twai_message_t->data_length_code;

    if (data[0] == 0x30)
    {
        handleQueryMotorPositionResponse(data, length);
    }
    else if (data[0] == 0xF5)
    {
        handleSetPositionResponse(data, length);
    }
    else if (data[0] == 0x31)
    {
        handleQueryStatusResponse(data, length);
    }
    else if (data[0] == 0x90)
    {
        handeSetHomeResponse(data, length);
    }
    else
    {
        ESP_LOGE(FUNCTION_NAME, "unimplemented code: %d", data[0]);

        ESP_LOGI(FUNCTION_NAME, "Raw code byte: %u", data[0]);

        for (int i = 0; i < length; i++)
        {
            ESP_LOGI(FUNCTION_NAME, "Data[%d]: %d", i, data[i]);
        }
    }
}

void MotorController::sendCommand(uint8_t *data, uint8_t length)
{

    uint8_t code = data[0];

    if (!code)
    {
        ESP_LOGE(FUNCTION_NAME, "Error: code is empty!");
        return;
    }

    char commandName[50];
    command_mapper->getCommandNameFromCode(code, commandName);

    ESP_LOGI(FUNCTION_NAME, "ID: %lu\t length: %u\t code: 0x%02X\t commandName: %s", canId, length, data[0], commandName);

    twai_message_t msg = {};
    msg.identifier = canId;
    msg.data_length_code = length + 1;
    msg.extd = 0;
    msg.rtr = 0;
    msg.ss = 0;
    msg.dlc_non_comp = 0;
    memcpy(&msg.data, data, length + 1);

    configASSERT(outQ);

    ESP_ERROR_CHECK(xQueueSendToBack(outQ, &msg, 0));
    ESP_LOGI(FUNCTION_NAME, "    ==> Message enqueued successfully!");
}

void MotorController::handleQueryStatusResponse(const uint8_t *data, uint8_t length)
{

    uint8_t status = data[1];

    switch (status)
    {
    case 0:
        ESP_LOGI(FUNCTION_NAME, "Abfrage fehlgeschlagen");
        break;
    case 1:
        ESP_LOGI(FUNCTION_NAME, "Motor gestoppt");
        stateMachine.setState(StateMachine::State::IDLE);
        break;
    case 2:
        ESP_LOGI(FUNCTION_NAME, "Motor beschleunigt");
        stateMachine.setState(StateMachine::State::MOVING);
        break;
    case 3:
        ESP_LOGI(FUNCTION_NAME, "Motor verlangsamt");
        stateMachine.setState(StateMachine::State::MOVING);
        break;
    case 4:
        ESP_LOGI(FUNCTION_NAME, "Motor volle Geschwindigkeit");
        stateMachine.setState(StateMachine::State::MOVING);
        break;
    case 5:
        ESP_LOGI(FUNCTION_NAME, "Motor wird geparkt");
        stateMachine.setState(StateMachine::State::MOVING);
        break;
    case 6:
        ESP_LOGI(FUNCTION_NAME, "Motor wird kalibriert");
        stateMachine.setState(StateMachine::State::MOVING);
        break;
    default:
        ESP_LOGI(FUNCTION_NAME, "Unbekannter Status");
        stateMachine.setState(StateMachine::State::ERROR);
        break;
    }
}

void MotorController::handleQueryMotorPositionResponse(const uint8_t *data, uint8_t length)
{

    if (length != 8 || data[0] != 0x30)
    {
        ESP_LOGE(FUNCTION_NAME, "Invalid response length or code.");
        return;
    }

    CarryValue = (data[1] << 24) | (data[2] << 16) | (data[3] << 8) | data[4];
    EncoderValue = (data[5] << 8) | data[6];

    ESP_LOGI(FUNCTION_NAME, "Carry value: %lu", CarryValue);
    ESP_LOGI(FUNCTION_NAME, "Encoder value: %u", EncoderValue);
}

void MotorController::handleSetPositionResponse(const uint8_t *data, uint8_t length)
{

    if (length != 3)
    {
        ESP_LOGE(FUNCTION_NAME, "Invalid response length.");
        return;
    }
    if (data[0] != 0xF5)
    {
        ESP_LOGE(FUNCTION_NAME, "Unexpected command code: %u", data[0]);
        return;
    }

    uint8_t status = data[1];

    std::string F5Status;
    switch (status)
    {
    case 0:
        F5Status = "Run failed";
        stateMachine.setState(StateMachine::State::ERROR);
        break;
    case 1:
        F5Status = "Run starting";
        stateMachine.setState(StateMachine::State::MOVING);
        break;
    case 2:
        F5Status = "Run complete";
        stateMachine.setState(StateMachine::State::COMPLETED);
        break;
    case 3:
        F5Status = "End limit stopped";
        stateMachine.setState(StateMachine::State::COMPLETED);
        break;
    default:
        F5Status = "Unknown status";
        stateMachine.setState(StateMachine::State::ERROR);
        break;
    }

    ESP_LOGI(FUNCTION_NAME, "Set Position Response: %s", F5Status.c_str());
}

void MotorController::handeSetHomeResponse(const uint8_t *data, uint8_t length)
{

    if (length != 3)
    {
        ESP_LOGE(FUNCTION_NAME, "Invalid response length: %u", length);
        return;
    }

    if (data[0] != 0x90)
    {
        ESP_LOGE(FUNCTION_NAME, "Unexpected command code: %u", data[0]);
        return;
    }

    uint8_t status = data[1];
    uint8_t crc = data[2];

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

void MotorController::handleSetWorkModeResponse(uint8_t *data, uint8_t length)
{

    if (data[1] == 1)
    {
        ESP_LOGI(FUNCTION_NAME, "Set Work Mode: Success");
    }
    else
    {
        ESP_LOGE(FUNCTION_NAME, "Set Work Mode: Failed");
    }
}

void MotorController::handleSetCurrentResponse(uint8_t *data, uint8_t length)
{

    if (data[1] == 1)
    {
        ESP_LOGI(FUNCTION_NAME, "Set Current: Success");
    }
    else
    {
        ESP_LOGE(FUNCTION_NAME, "Set Current: Failed");
    }
}
