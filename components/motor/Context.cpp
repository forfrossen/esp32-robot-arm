#include "Context.hpp"
ESP_EVENT_DEFINE_BASE(PROPERTY_CHANGE_EVENTS);

void MotorContext::transition_ready_state(ReadyState new_state)
{
    // xSemaphoreTake(motor_mutex, portMAX_DELAY);
    if (xSemaphoreTake(context_mutex, portMAX_DELAY) == pdTRUE)
    {
        ESP_LOGI(FUNCTION_NAME, "Transitioning from %s to %s", magic_enum::enum_name(ready_state).data(), magic_enum::enum_name(new_state).data());
        ready_state = new_state;
        post_new_state_event();

        xSemaphoreGive(context_mutex);
    }
    else
    {
        ESP_LOGE(FUNCTION_NAME, "Failed to take mutex");
    }
}

template <typename T>
T MotorContext::get_property(T MotorProperties::*property) const
{
    return properties[property];
}

template <typename T>
void MotorContext::set_property(T MotorProperties::*property, T value)
{
    if (xSemaphoreTake(context_mutex, portMAX_DELAY) == pdTRUE)
    {
        ESP_LOGI(FUNCTION_NAME, "Set property %s to %d", magic_enum::enum_name(property), value);
        properties[property] = value;
        post_property_change_event(property, value);
        xSemaphoreGive(context_mutex);
    }
    else
    {
        ESP_LOGE(FUNCTION_NAME, "Failed to take mutex");
    }
}

template <typename T>
esp_err_t MotorContext::post_property_change_event(T MotorProperties::*property, const T &value)
{
    ESP_LOGI(FUNCTION_NAME, "Posting to PROPERTY_CHANGE_EVENTS");

    std::unique_ptr<MotorPropertyChangeEventData<T>> data = std::make_unique(property, value);

    esp_event_post_to(system_event_loop, SYSTEM_EVENTS, PROPERTY_CHANGE_EVENT, (void *)data.get(), sizeof(data), portMAX_DELAY);

    return ESP_OK;
}

void MotorContext::post_new_state_event()
{
    ESP_LOGI(FUNCTION_NAME, "Posting to MOTOR_EVENTS: %d \t %s", static_cast<int>(ready_state), magic_enum::enum_name(ready_state).data());

    ESP_ERROR_CHECK(esp_event_post_to(
        motor_event_loop,
        MOTOR_EVENTS,
        STATE_TRANSITION_EVENT,
        (void *)&ready_state,
        sizeof(ReadyState),
        portMAX_DELAY));

    ESP_LOGI(FUNCTION_NAME, "Posted to MOTOR_EVENTS");
}
