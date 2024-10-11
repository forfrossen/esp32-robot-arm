#include "Context.hpp"
// ESP_EVENT_DEFINE_BASE(PROPERTY_CHANGE_EVENTS);

void MotorContext::transition_ready_state(ReadyState new_state)
{
    // xSemaphoreTake(motor_mutex, portMAX_DELAY);

    if (ready_state == new_state)
    {
        return;
    }
    post_motor_event(new_state);

    // xSemaphoreGive(motor_mutex);

    ready_state = new_state;
}

template <typename T>
T MotorContext::get_property(T MotorProperties::*property) const
{
    return properties[property];
}

template <typename T>
void MotorContext::set_property(T MotorProperties::*property, T value)
{
    ESP_LOGI(FUNCTION_NAME, "Set property %s to %d", magic_enum::enum_name(property), value);
    properties[property] = value;
    post_property_change_event(property, value);
}

template <typename T>
esp_err_t MotorContext::post_property_change_event(T MotorProperties::*property, const T &value)
{
    ESP_LOGI(FUNCTION_NAME, "Posting to PROPERTY_CHANGE_EVENTS");

    MotorPropertyChangeEventData<T> data(property, value);

    esp_event_post_to(system_event_loop, SYSTEM_EVENTS, PROPERTY_CHANGE_EVENT, &data, sizeof(data), portMAX_DELAY);

    return ESP_OK;
}

void MotorContext::post_motor_event(ReadyState event)
{
    esp_event_post_to(motor_event_loop, MOTOR_EVENTS, STATE_TRANSITION_EVENT, &event, sizeof(ReadyState), portMAX_DELAY);
}
