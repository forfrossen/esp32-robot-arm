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
    case MOTOR_INIT:
        ESP_LOGI("Motor", "Motor is initializing.");
        post_event(MotorEvent::MOTOR_EVENT_INIT);
        break;
    case MOTOR_READY:
        ESP_LOGI("Motor", "Motor is now ready.");
        post_event(MotorEvent::MOTOR_EVENT_READY);
        break;
    case MOTOR_ERROR:
        ESP_LOGE("Motor", "Motor encountered an error.");
        post_event(MotorEvent::MOTOR_EVENT_ERROR);
        break;
    case MOTOR_RECOVERING:
        ESP_LOGI("Motor", "Motor is recovering from error.");
        post_event(MotorEvent::MOTOR_EVENT_RECOVERING);
        break;
    default:
        break;
    }
    // xSemaphoreGive(motor_mutex);

    ready_state = new_state;
}

void MotorContext::post_event(MotorEvent event)
{
    esp_event_post_to(event_group, MOTOR_CONTROLLER_EVENT, event, NULL, 0, portMAX_DELAY);
}