#include "Context.hpp"

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
    ESP_LOGD(TAG, "Transitioning MotorContext.ready_state: | %s | ==> | %s |",
             magic_enum::enum_name(ready_state).data(),
             magic_enum::enum_name(new_state).data());
    ready_state = new_state;
    xSemaphoreGive(context_mutex);
    CHECK_THAT(post_new_state_event() == ESP_OK);
    return ESP_OK;
}

esp_err_t MotorContext::post_new_state_event()
{
    ESP_LOGD(TAG, "Posting to MOTOR_EVENTS: %d \t %s", static_cast<int>(ready_state), magic_enum::enum_name(ready_state).data());

    ESP_ERROR_CHECK(esp_event_post_to(
        motor_event_loop,
        MOTOR_EVENTS,
        STATE_TRANSITION_EVENT,
        (void *)&ready_state,
        sizeof(ReadyState),
        portMAX_DELAY));

    ESP_LOGD(TAG, "Posted to MOTOR_EVENTS");
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
//         ESP_LOGD(TAG, "Setting property to value");

//         (properties.*property).set(value);

//         // post_property_change_event(property, value);

//         xSemaphoreGive(context_mutex);
//     }
//     else
//     {
//         ESP_LOGE(TAG, "Failed to take mutex");
//         return ESP_FAIL;
//     }

//     return ESP_OK;
// }

esp_err_t MotorContext::post_property_change_event(const std::string &property_name, const void *value_ptr, PayloadType type)
{
    // Prepare the event data
    PropertyChangeEventData event_data{};
    event_data.property_name = property_name;
    event_data.type = type;

    // Copy the value based on the type
    switch (type)
    {
    case PayloadType::UINT4:
    case PayloadType::UINT8:
        event_data.value.uint8_value = *reinterpret_cast<const uint8_t *>(value_ptr);
        break;
    case PayloadType::UINT16:
        event_data.value.uint16_value = *reinterpret_cast<const uint16_t *>(value_ptr);
        break;
    case PayloadType::INT16:
        event_data.value.int16_value = *reinterpret_cast<const int16_t *>(value_ptr);
        break;
    case PayloadType::INT24:
    case PayloadType::INT32:
        event_data.value.int32_value = *reinterpret_cast<const int32_t *>(value_ptr);
        break;
    case PayloadType::UINT24:
    case PayloadType::UINT32:
        event_data.value.uint32_value = *reinterpret_cast<const uint32_t *>(value_ptr);
        break;
    case PayloadType::UINT48:
        event_data.value.uint48_value = *reinterpret_cast<const uint64_t *>(value_ptr);
        break;
    case PayloadType::CHRONO:
        event_data.value.chrono_value = *reinterpret_cast<const std::chrono::system_clock::time_point *>(value_ptr);
        break;

    case PayloadType::VOID:
    default:
        ESP_LOGE("post_property_change_event", "Unsupported PayloadType for property: %s", property_name.c_str());
        return ESP_FAIL;
    }

    ESP_LOGD(TAG, "Posting to PROPERTY_CHANGE_EVENTS");

    CHECK_THAT(esp_event_post_to(
                   system_event_loop,
                   SYSTEM_EVENTS,
                   PROPERTY_CHANGE_EVENT,
                   &event_data,
                   sizeof(event_data),
                   portMAX_DELAY) == ESP_OK);

    return ESP_OK;
}