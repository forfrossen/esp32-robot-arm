#ifndef MOTORCONTEXT_HPP
#define MOTORCONTEXT_HPP

#include "TypeDefs.hpp"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <chrono>
#include <cstdint>

class MotorContext
{
public:
    enum ReadyState
    {
        MOTOR_INIT,
        MOTOR_READY,
        MOTOR_ERROR,
        MOTOR_RECOVERING
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

    MotorContext(uint32_t can_id) : can_id(can_id) {};
    ~MotorContext() {}

    uint32_t get_carry_value() const { return carry_value; };
    void set_carry_value(uint32_t value) { carry_value = value; };

    uint16_t get_encoder_value() const { return encoder_value; };
    void set_encoder_value(uint16_t value) { encoder_value = value; };

    MovingState get_motor_moving_state() const { return moving_state; };
    void set_motor_moving_state(MovingState state) { moving_state = state; };

    std::chrono::system_clock::time_point get_last_seen() const { return last_seen; };
    void set_last_seen(std::chrono::system_clock::time_point time) { last_seen = time; };

    ReadyState get_ready_state() const { return ready_state; };

    void transition_ready_state(ReadyState new_state);

    bool is_ready() const { return ready_state == ReadyState::MOTOR_READY; };
    bool is_recovering() const { return ready_state == ReadyState::MOTOR_RECOVERING; };
    bool is_init() const { return ready_state == ReadyState::MOTOR_INIT; };
    bool is_error() const { return ready_state == ReadyState::MOTOR_ERROR; };

private:
    uint32_t can_id;
    ReadyState ready_state = ReadyState::MOTOR_INIT;
    MovingState moving_state = MovingState::UNKNOWN;
    std::chrono::system_clock::time_point last_seen;
    uint32_t carry_value;
    uint16_t encoder_value;

    EventGroupHandle_t event_group;

    void post_event(MotorEvent event);
};

#endif // MOTORCONTEXT_HPP