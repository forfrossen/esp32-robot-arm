#include "ResponseHandler.hpp"

ResponseHandler::~ResponseHandler() {}

void ResponseHandler::process_message(twai_message_t *msg)
{
    log_twai_message(msg);
    check_for_error_and_do_transition(msg);

    switch (msg->data[0])
    {
    case 0x30:
        handle_query_motor_position_response(msg);
        break;
    case 0x82:
        handle_set_work_mode_response(msg);
        break;
    case 0x83:
        handle_set_current_response(msg);
        break;
    case 0x90:
        handle_set_home_response(msg);
        break;
    case 0xF1:
        handle_query_status_response(msg);
        break;
    case 0xF4:
    case 0xF5:
        handle_set_position_response(msg);
        break;
    default:
        ESP_LOGW(FUNCTION_NAME, "No response handler defined for code: %02X", msg->data[0]);
        break;
    }
}

void ResponseHandler::log_twai_message(twai_message_t *msg)
{

    ESP_LOGI(FUNCTION_NAME, "ID: 0x%02lu \t length: %d / %02u \t code: 0x%02X \t commandName: %s", canId, msg->data_length_code, msg->data_length_code, msg->data[0], GET_CMD_NAME(GET_CMD(msg)).c_str());

    for (int i = 0; i < msg->data_length_code - 1; i++)
    {
        ESP_LOGI(FUNCTION_NAME, "  Data[%d]: \t 0x%02X \t %d ", i, msg->data[i], msg->data[i]);
    }
    ESP_LOGI(FUNCTION_NAME, "  Data CRC: \t 0x%02X \t %d ", msg->data[msg->data_length_code], msg->data[msg->data_length_code]);
}

bool ResponseHandler::is_response_error(twai_message_t *msg)
{
    if (msg->data[1] == 0)
    {
        return true;
    }
    return false;
}
void ResponseHandler::handle_received_no_error()
{
    if (context->is_error())
    {
        return context->transition_ready_state(MotorContext::ReadyState::MOTOR_RECOVERING);
    }
    if (context->is_init())
    {
        return context->transition_ready_state(MotorContext::ReadyState::MOTOR_RECOVERING);
    }
}

void ResponseHandler::handle_message_error()
{
    context->transition_ready_state(MotorContext::ReadyState::MOTOR_ERROR);
}

void ResponseHandler::check_for_error_and_do_transition(twai_message_t *msg)
{
    if (is_response_error(msg))
    {
        handle_message_error();
        ESP_LOGI(FUNCTION_NAME, "Error response received.");
    }
    else
    {
        handle_received_no_error();
        ESP_LOGI(FUNCTION_NAME, "Response does not contain an error.");
    }
}

void ResponseHandler::print_unknown_response_code(twai_message_t *msg)
{
    auto status = msg->data[1];
    ESP_LOGW(FUNCTION_NAME, "Unknown status response: %02X", status);
    for (int i = 0; i < msg->data_length_code; i++)
    {
        ESP_LOGI(FUNCTION_NAME, "Data[%d] - raw: %d \t - hex: %02X", i, msg->data[i], msg->data[i]);
    }
}

void ResponseHandler::handle_query_status_response(twai_message_t *msg)
{
    uint8_t status = msg->data[1];

    switch (status)
    {
    case 0:
        ESP_LOGE(FUNCTION_NAME, "Abfrage fehlgeschlagen");
        context->set_motor_moving_state(MotorContext::MovingState::UNKNOWN);
        break;
    case 1:
        ESP_LOGI(FUNCTION_NAME, "Motor gestoppt");
        context->set_motor_moving_state(MotorContext::MovingState::STOPPED);
        break;
    case 2:
        ESP_LOGI(FUNCTION_NAME, "Motor beschleunigt");
        context->set_motor_moving_state(MotorContext::MovingState::ACCELERATING);
        break;
    case 3:
        ESP_LOGI(FUNCTION_NAME, "Motor verlangsamt");
        context->set_motor_moving_state(MotorContext::MovingState::DECELERATING);
        break;
    case 4:
        ESP_LOGI(FUNCTION_NAME, "Motor volle Geschwindigkeit");
        context->set_motor_moving_state(MotorContext::MovingState::FULL_SPEED);
        break;
    case 5:
        ESP_LOGI(FUNCTION_NAME, "Motor fährt nach Hause");
        context->set_motor_moving_state(MotorContext::MovingState::HOMING);
        break;
    case 6:
        ESP_LOGI(FUNCTION_NAME, "Motor wird kalibriert");
        context->set_motor_moving_state(MotorContext::MovingState::CALIBRATING);
        break;
    default:
        ESP_LOGW(FUNCTION_NAME, "Unknown status response: %02X", status);
        context->set_motor_moving_state(MotorContext::MovingState::UNKNOWN);
        break;
    }
}

void ResponseHandler::handle_query_motor_position_response(twai_message_t *msg)
{
    uint8_t *data = msg->data;

    uint32_t carry = (data[1] << 24) | (data[2] << 16) | (data[3] << 8) | data[4];
    uint16_t encoderValue = (data[5] << 8) | data[6];

    // Optional: Überprüfen, ob Value außerhalb des gültigen Bereichs liegt
    if (encoderValue > 0x3FFF)
    {
        carry += 1;
        encoderValue -= 0x4000;
    }
    else if (encoderValue < 0)
    {
        carry -= 1;
        encoderValue += 0x4000;
    }

    // Werte im Kontext speichern
    context->set_carry_value(carry);
    context->set_encoder_value(encoderValue);

    // Absolute Position berechnen
    int64_t absolutePosition = ((int64_t)carry << 14) + encoderValue;
    context->set_absolute_position(absolutePosition);

    ESP_LOGI(FUNCTION_NAME, "Carry value: %ld", carry);
    ESP_LOGI(FUNCTION_NAME, "Encoder value: %u", encoderValue);
    ESP_LOGI(FUNCTION_NAME, "Absolute position: %lld", absolutePosition);
}

void ResponseHandler::handle_query_motor_speed_response(twai_message_t *msg)
{
    uint16_t speed = (msg->data[2] << 16) | (msg->data[3] << 8) | msg->data[4];
    uint8_t crc = msg->data[3];

    ESP_LOGI(FUNCTION_NAME, "Speed: %u", speed);
}

void ResponseHandler::handle_set_position_response(twai_message_t *msg)
{
    uint8_t status = msg->data[1];

    std::string F5Status;
    switch (status)
    {
    case 0:
        F5Status = "Run failed";
        ESP_LOGI(FUNCTION_NAME, "Run failed");
        // context->set_motor_moving_state(MotorContext::MovingState::ERROR);
        break;
    case 1:
        F5Status = "Run starting";
        ESP_LOGI(FUNCTION_NAME, "Run starting");
        // fsm_moving.set_state(MotorContext::ReadyState::);
        break;
    case 2:
        F5Status = "Run complete";
        ESP_LOGI(FUNCTION_NAME, "Run complete");
        // fsm_moving.set_state(MotorControllerFSM::State::COMPLETED);
        break;
    case 3:
        F5Status = "End limit stopped";
        ESP_LOGI(FUNCTION_NAME, "End limit stopped");
        // fsm_moving.set_state(MotorControllerFSM::State::COMPLETED);
        break;
    default:
        F5Status = "Unknown status";
        ESP_LOGW(FUNCTION_NAME, "Unknown status");
        // fsm_moving.set_state(MotorControllerFSM::State::ERROR);
        break;
    }
}

void ResponseHandler::handle_set_home_response(twai_message_t *msg)
{
    uint8_t status = msg->data[1];
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
}

void ResponseHandler::handle_set_work_mode_response(twai_message_t *msg)
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

void ResponseHandler::handle_set_current_response(twai_message_t *msg)
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
