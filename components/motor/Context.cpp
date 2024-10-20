#include "Context.hpp"

ESP_EVENT_DEFINE_BASE(PROPERTY_CHANGE_EVENTS);

esp_err_t MotorContext::get_semaphore()
{
    CHECK_THAT(xSemaphoreTake(context_mutex, portMAX_DELAY) == pdTRUE);
    return ESP_OK;
}

esp_err_t MotorContext::transition_ready_state(ReadyState new_state)
{
    if (new_state == ready_state)
    {
        return ESP_OK;
    }
    CHECK_THAT(get_semaphore() == ESP_OK);
    ESP_LOGI(FUNCTION_NAME, "Transitioning MotorContext.ready_state: | %s | ==> | %s |",
             magic_enum::enum_name(ready_state).data(),
             magic_enum::enum_name(new_state).data());
    ready_state = new_state;
    xSemaphoreGive(context_mutex);
    CHECK_THAT(post_new_state_event() == ESP_OK);
    return ESP_OK;
}

esp_err_t MotorContext::post_new_state_event()
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
    return ESP_OK;
}

// // Explicit template instantiations for primitive types
// template esp_err_t MotorContext::set_property<uint8_t>(MotorProperty<uint8_t> MotorProperties::*, uint8_t);
// template esp_err_t MotorContext::set_property<uint16_t>(MotorProperty<uint16_t> MotorProperties::*, uint16_t);
// template esp_err_t MotorContext::set_property<uint32_t>(MotorProperty<uint32_t> MotorProperties::*, uint32_t);
// template esp_err_t MotorContext::set_property<uint64_t>(MotorProperty<uint64_t> MotorProperties::*, uint64_t);
// template esp_err_t MotorContext::set_property<int16_t>(MotorProperty<int16_t> MotorProperties::*, int16_t);
// template esp_err_t MotorContext::set_property<int32_t>(MotorProperty<int32_t> MotorProperties::*, int32_t);
// template esp_err_t MotorContext::set_property<int64_t>(MotorProperty<int64_t> MotorProperties::*, int64_t);

// // Explicit template instantiations for enum class types
// template esp_err_t MotorContext::set_property<Direction>(MotorProperty<Direction> MotorProperties::*, Direction);
// template esp_err_t MotorContext::set_property<EnPinEnable>(MotorProperty<EnPinEnable> MotorProperties::*, EnPinEnable);
// template esp_err_t MotorContext::set_property<Enable>(MotorProperty<Enable> MotorProperties::*, Enable);
// template esp_err_t MotorContext::set_property<EnableStatus>(MotorProperty<EnableStatus> MotorProperties::*, EnableStatus);
// template esp_err_t MotorContext::set_property<MotorStatus>(MotorProperty<MotorStatus> MotorProperties::*, MotorStatus);
// template esp_err_t MotorContext::set_property<RunMotorResult>(MotorProperty<RunMotorResult> MotorProperties::*, RunMotorResult);
// template esp_err_t MotorContext::set_property<MotorShaftProtectionStatus>(MotorProperty<MotorShaftProtectionStatus> MotorProperties::*, MotorShaftProtectionStatus);
// template esp_err_t MotorContext::set_property<Mode0>(MotorProperty<Mode0> MotorProperties::*, Mode0);
// template esp_err_t MotorContext::set_property<SaveCleanState>(MotorProperty<SaveCleanState> MotorProperties::*, SaveCleanState);
// template esp_err_t MotorContext::set_property<CalibrationResult>(MotorProperty<CalibrationResult> MotorProperties::*, CalibrationResult);
// template esp_err_t MotorContext::set_property<EndStopLevel>(MotorProperty<EndStopLevel> MotorProperties::*, EndStopLevel);
// template esp_err_t MotorContext::set_property<CanBitrate>(MotorProperty<CanBitrate> MotorProperties::*, CanBitrate);

// esp_err_t MotorContext::set_meta_property(MotorPropertyVariant MotorProperties::*property, MotorPropertyVariant value)
// {
//     CHECK_THAT(get_semaphore() == ESP_OK);

//     if (xSemaphoreTake(context_mutex, portMAX_DELAY) == pdTRUE)
//     {
//         // Log the property and value with a type-generic approach
//         ESP_LOGI(FUNCTION_NAME, "Setting property to value");

//         (properties.*property).set(value);

//         // post_property_change_event(property, value);

//         xSemaphoreGive(context_mutex);
//     }
//     else
//     {
//         ESP_LOGE(FUNCTION_NAME, "Failed to take mutex");
//         return ESP_FAIL;
//     }

//     return ESP_OK;
// }

// esp_err_t MotorContext::post_property_change_event(MotorPropertyVariant MotorProperties::*property, const MotorPropertyVariant &value)
// {
//     ESP_LOGI(FUNCTION_NAME, "Posting to PROPERTY_CHANGE_EVENTS");

//     // std::unique_ptr<MotorPropertyChangeEventData<T>> data = std::make_unique(property, value);

//     esp_event_post_to(system_event_loop, SYSTEM_EVENTS, PROPERTY_CHANGE_EVENT, (void *)data.get(), sizeof(data), portMAX_DELAY);

//     return ESP_OK;
// }