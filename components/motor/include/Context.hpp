#ifndef MOTORCONTEXT_HPP
#define MOTORCONTEXT_HPP

#include "Events.hpp"
#include "MksEnums.hpp"
#include "Properties.hpp"
#include "TypeDefs.hpp"
#include "utils.hpp"
#include <chrono>
#include <cstdint>
#include <esp_err.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_random.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <magic_enum.hpp>
#include <variant>

class MotorContext
{
public:
    enum class ReadyState
    {
        MOTOR_UNINITIALIZED,
        MOTOR_INITIALIZING,
        MOTOR_READY,
        MOTOR_ERROR,
        MOTOR_RECOVERING,
        UNKNOWN
    };

    enum class MovingState
    {
        UNKNOWN,
        ERROR,
        STOPPED,
        ACCELERATING,
        DECELERATING,
        FULL_SPEED,
        HOMING,
        CALIBRATING
    };

    MotorContext(uint32_t can_id,
                 std::shared_ptr<EventLoops> event_loops)
        : can_id(can_id),
          context_mutex(xSemaphoreCreateMutex()),
          system_event_loop(event_loops->system_event_loop),
          motor_event_loop(event_loops->motor_event_loop) {};

    ~MotorContext() {}

    MovingState get_motor_moving_state() const { return moving_state; };
    void set_motor_moving_state(MovingState state) { moving_state = state; };

    std::chrono::system_clock::time_point get_last_seen() const { return last_seen; };
    // esp_err_t set_last_seen(std::chrono::system_clock::time_point time) { last_seen = time; };

    ReadyState get_ready_state() const { return ready_state; };
    esp_err_t transition_ready_state(ReadyState new_state);

    bool is_ready() const { return ready_state == ReadyState::MOTOR_READY; };
    bool is_recovering() const { return ready_state == ReadyState::MOTOR_RECOVERING; };
    bool is_init() const { return ready_state == ReadyState::MOTOR_INITIALIZING; };
    bool is_error() const { return ready_state == ReadyState::MOTOR_ERROR; };

    esp_err_t set_property_value(const ResponsePropertyMetadata &metadata, const uint8_t *data);

    MotorProperties &get_properties() { return properties; };

    // Optionally, you can add a generic getter method
    template <typename T>
    T get_property(T MotorProperties::*property) const;

private:
    uint32_t can_id;
    ReadyState ready_state = ReadyState::MOTOR_UNINITIALIZED;
    MovingState moving_state = MovingState::UNKNOWN;
    std::chrono::system_clock::time_point last_seen;

    MotorProperties properties;

    SemaphoreHandle_t context_mutex;

    esp_event_loop_handle_t system_event_loop;
    esp_event_loop_handle_t motor_event_loop;

    esp_err_t post_new_state_event();
    esp_err_t post_property_change_event(const std::string &property_name, const void *value_ptr, PayloadType type);
    esp_err_t get_semaphore();
};

template <typename T>
T MotorContext::get_property(T MotorProperties::*property) const
{
    return properties.*property;
}
#endif // MOTORCONTEXT_HPP
