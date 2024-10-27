#include "Controller.hpp"

/**
 * Event handler for motor state transitions
 */
void MotorController::on_state_transition(void *handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    MotorController *instance = static_cast<MotorController *>(handler_arg);
    esp_err_t ret = ESP_OK;

    RETURN_VOID_IF(instance == nullptr);
    RETURN_VOID_IF(event_base != MOTOR_EVENTS);
    RETURN_VOID_IF(event_id != STATE_TRANSITION_EVENT);

    ESP_LOGD(TAG, "GOT MOTOR EVENT FOR MOTOR %lu, EVENT_BASE: %s, EVENT_ID: %lu", instance->canId, event_base, event_id);

    MotorContext::ReadyState *new_state = (MotorContext::ReadyState *)event_data;
    const char *state_name = magic_enum::enum_name(*new_state).data();
    state_name == nullptr && (state_name = "UNKNOWN");

    ESP_LOGD(TAG, "TRANSITION_STATE: %s", state_name);

    switch (*new_state)
    {

    case MotorContext::ReadyState::MOTOR_RECOVERING:
        ESP_LOGD(TAG, "Handling MOTOR_EVENT_RECOVERING.");
        ret = instance->handle_recover();
        break;

    case MotorContext::ReadyState::MOTOR_INITIALIZED:
        ESP_LOGD(TAG, "Handling MOTOR_EVENT_INIT.");
        ret = instance->handle_initialize();
        break;

    case MotorContext::ReadyState::MOTOR_ERROR:
        ESP_LOGE(TAG, "Handling MOTOR_EVENT_ERROR.");
        ret = instance->handle_error();
        break;

    case MotorContext::ReadyState::MOTOR_READY:
        ESP_LOGD(TAG, "Handling MOTOR_EVENT_READY.");
        ret = instance->handle_ready();
        break;

    default:
        break;
    }

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Error handling state transition event: %s", esp_err_to_name(ret));
    }
}

/**
 * Task to query the motor position
 */
void MotorController::vTask_query_position(void *pvParameters)
{
    MotorController *instance = static_cast<MotorController *>(pvParameters);
    for (;;)
    {
        ESP_LOGD(TAG, "New iteration of taskQueryMotorPosition");
        if (xEventGroupWaitBits(instance->motor_event_group, MOTOR_READY_BIT, pdFALSE, pdFALSE, portMAX_DELAY) & MOTOR_READY_BIT)
        {
            ESP_LOGD(TAG, "New iteration of taskQueryMotorPosition");
            instance->query_position();
        }

        vTaskDelay(4000 / portTICK_PERIOD_MS);
    }
}

/**
 * Task to query the motor status
 */
void MotorController::vTask_query_status(void *pvParameters)
{
    MotorController *instance = static_cast<MotorController *>(pvParameters);
    int local_error_counter = 0;
    for (;;)
    {
        ESP_LOGD(TAG, "New iteration of vTask_query_status");
        esp_err_t ret = instance->query_status();
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "Error enqueing query-status-command. Error counter: %d", local_error_counter);
            local_error_counter < 5 && local_error_counter++;
        }
        else
        {
            local_error_counter = 0;
            ESP_LOGD(TAG, "Query status command enqueued successfully");
        }

        vTaskDelay((4000 * (local_error_counter + 1)) / portTICK_PERIOD_MS);
    }
}

/**
 * Task to send random target position commands
 */
void MotorController::vtask_send_positon(void *pvParameters)
{
    MotorController *instance = static_cast<MotorController *>(pvParameters);
    for (;;)
    {
        if (xEventGroupWaitBits(instance->motor_event_group, MOTOR_READY_BIT, pdFALSE, pdFALSE, portMAX_DELAY) & MOTOR_READY_BIT)
        {
            ESP_LOGD(TAG, "New iteration of taskSendRandomTargetPositionCommands");

            bool is_ready_for_command = false;
            do
            {
                if (instance->context == nullptr)
                {
                    ESP_LOGE(TAG, "MotorController context is null");
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                    continue;
                }

                MotorStatus motor_status = static_cast<MotorStatus>(instance->context->get_property(&MotorProperties::motor_status));
                auto motor_status_name = magic_enum::enum_name(motor_status);

                if (motor_status_name.empty())
                {
                    ESP_LOGE(TAG, "Invalid MotorStatus value: %d", static_cast<int>(motor_status));
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                    continue;
                }

                ESP_LOGD(TAG, "Waiting for Motor to be in stopped state. Currently is: %s", motor_status_name.data());
                if (motor_status == MotorStatus::MotorStop)
                {
                    is_ready_for_command = true;
                }
                else
                {
                    ESP_LOGW(TAG, "Motor is not in stopped state. Waiting for 1s");
                }
                vTaskDelay(1000 / portTICK_PERIOD_MS);
            } while (!is_ready_for_command);

            instance->set_target_position();
        }

        vTaskDelay(6000 / portTICK_PERIOD_MS);
    }
}