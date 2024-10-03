#include "MotorContext.hpp"
void MotorContext::transition_ready_state(ReadyState new_state)
{
    // xSemaphoreTake(motor_mutex, portMAX_DELAY);

    if (ready_state == new_state)
    {
        return;
    }

    switch (new_state)
    {
    case MOTOR_INITIALIZED:
        ESP_LOGI(FUNCTION_NAME, "Motor is initializing.");
        post_event(motor_event_id_t::MOTOR_EVENT_INIT);
        break;
    case MOTOR_ERROR:
        ESP_LOGE(FUNCTION_NAME, "Motor encountered an error.");
        post_event(motor_event_id_t::MOTOR_EVENT_ERROR);
        break;
    case MOTOR_RECOVERING:
        ESP_LOGI(FUNCTION_NAME, "Motor is recovering from error.");
        post_event(motor_event_id_t::MOTOR_EVENT_RECOVERING);
        break;
    case MOTOR_READY:
        ESP_LOGI(FUNCTION_NAME, "Motor is now ready.");
        post_event(motor_event_id_t::MOTOR_EVENT_READY);
        break;
    default:
        break;
    }
    // xSemaphoreGive(motor_mutex);

    ready_state = new_state;
}

void MotorContext::post_event(motor_event_id_t event)
{
    ESP_LOGI(FUNCTION_NAME, "Posting to motor_event_loop with base: MOTOR_EVENT the event: %d ", event);
    esp_event_post_to(motor_event_loop, MOTOR_EVENT, event, NULL, 0, portMAX_DELAY);
}