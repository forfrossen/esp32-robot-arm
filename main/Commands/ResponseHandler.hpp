#ifndef RESPONSEHANDLER_H
#define RESPONSEHANDLER_H

#include "../CommandMapper.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "utils.hpp"
#include <driver/twai.h>

class ResponseHandler
{
public:
    twai_message_t msg = {};

    ResponseHandler(twai_message_t *msg) : msg(*msg)
    {
    }

    ~ResponseHandler()
    {
    }

    esp_err_t handle_received_message(twai_message_t *msg)
    {
        esp_err_t ret = ESP_OK;

        if (!msg->data[0])
        {
            ESP_LOGE(FUNCTION_NAME, "Error: code is empty!");
            return ret = ESP_FAIL;
        }

        char commandName[50];
        command_mapper->get_command_name_from_code(msg->data[0], commandName);

        ESP_LOGI(FUNCTION_NAME, "ID: %lu\t length: %u\tcode: %d\tcommandName: %s", canId, msg->data_length_code, msg->data[0], commandName);

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

    void handle_query_status_response(twai_message_t *msg)
    {
        uint8_t status = msg->data[1];

        switch (status)
        {
        case 0:
            ESP_LOGI(FUNCTION_NAME, "Abfrage fehlgeschlagen");
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
            motor_moving_state = MotorMovingState::UNKNOWN;
            break;
        }
    }

    void handleQueryMotorPositionResponse(twai_message_t *msg)
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

    void handleSetPositionResponse(twai_message_t *msg)
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
            state_machine.set_state(StateMachine::State::ERROR);
            break;
        case 1:
            F5Status = "Run starting";
            state_machine.set_state(StateMachine::State::MOVING);
            break;
        case 2:
            F5Status = "Run complete";
            state_machine.set_state(StateMachine::State::COMPLETED);
            break;
        case 3:
            F5Status = "End limit stopped";
            state_machine.set_state(StateMachine::State::COMPLETED);
            break;
        default:
            F5Status = "Unknown status";
            state_machine.set_state(StateMachine::State::ERROR);
            break;
        }

        ESP_LOGI(FUNCTION_NAME, "Set Position Response: %s", F5Status.c_str());
    }

    void handeSetHomeResponse(twai_message_t *msg)
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

    void handleSetWorkModeResponse(twai_message_t *msg)
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

    void handleSetCurrentResponse(twai_message_t *msg)
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

private:
    CommandMapper *command_mapper();
};
#endif