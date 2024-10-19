#ifndef MOTORCONTEXT_HPP
#define MOTORCONTEXT_HPP

#include "../magic_enum/include/magic_enum/magic_enum.hpp"
#include "Events.hpp"
#include "MksEnums.hpp"
#include "Properties.hpp"
#include "TypeDefs.hpp"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "utils.hpp"
#include <chrono>
#include <cstdint>
#include <variant>

template <typename T>
struct MotorPropertyChangeEventData
{
    T MotorProperties::*property;
    T value;
    MotorPropertyChangeEventData(T MotorProperties::*property, T value) : property(property), value(value) {}
};

class MotorContext
{
public:
    enum class ReadyState
    {
        MOTOR_UNINITIALIZED,
        MOTOR_INITIALIZED,
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

    MotorContext(uint32_t can_id, std::shared_ptr<EventLoops> event_loops) : can_id(can_id),
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
    bool is_init() const { return ready_state == ReadyState::MOTOR_INITIALIZED; };
    bool is_error() const { return ready_state == ReadyState::MOTOR_ERROR; };

    esp_err_t set_meta_property(MotorPropertyVariant MotorProperties::*property, MotorPropertyVariant value);
    esp_err_t set_property(MotorPropertyVariant MotorProperties::*property, MotorPropertyVariant value);
    esp_err_t set_property_from_variant(MotorPropertyVariant &property, const MotorPropertyVariant &value);

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

    template <typename T>
    esp_err_t post_property_change_event(T MotorProperties::*property, const T &value);

    esp_err_t get_semaphore();
};

#endif // MOTORCONTEXT_HPP

/* // Macro to create a setter
#define CREATE_SETTER(type, name) \
    void set_##name(type value) { properties.name = value; }

// Macro to create a getter
#define CREATE_GETTER(type, name) \
    type get_##name() const { return properties.name; }

// Macro to create both setter and getter
#define CREATE_PROPERTY(type, name) \
    CREATE_GETTER(type, name)       \
    CREATE_SETTER(type, name)

    // Automatically generate getters and setters using the macro
    CREATE_PROPERTY(uint16_t, working_current)
    CREATE_PROPERTY(uint16_t, holding_current)
    CREATE_PROPERTY(uint8_t, motor_rotation_direction)
    CREATE_PROPERTY(uint8_t, subdivisions)
    CREATE_PROPERTY(uint8_t, en_pin_config)
    CREATE_PROPERTY(bool, key_lock_enabled)
    CREATE_PROPERTY(bool, auto_turn_off_screen)
    CREATE_PROPERTY(bool, locked_rotor_protection)
    CREATE_PROPERTY(uint8_t, can_id)
    CREATE_PROPERTY(uint8_t, can_bitrate)
    CREATE_PROPERTY(uint8_t, group_id)
    CREATE_PROPERTY(uint8_t, home_trig)
    CREATE_PROPERTY(uint8_t, home_dir)
    CREATE_PROPERTY(uint16_t, home_speed)
    CREATE_PROPERTY(uint8_t, end_limit)
    CREATE_PROPERTY(bool, emergency_stop_triggered)
    CREATE_PROPERTY(bool, is_enabled)
    CREATE_PROPERTY(int32_t, current_position)
    CREATE_PROPERTY(int32_t, target_position)
    CREATE_PROPERTY(float, current_speed)
    CREATE_PROPERTY(bool, is_moving) */
