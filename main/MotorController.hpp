#ifndef MOTORCONTROLLER_H
#define MOTORCONTROLLER_H

#include "CommandMapper.hpp"
#include "Commands/TWAICommandFactory.hpp"
#include "StateMachine.hpp"
#include "TWAIController.hpp"
#include "TypeDefs.hpp"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "utils.hpp"
#include <chrono>
#include <cstdint>
#include <functional>
#include <map>

#define MAX_PROCESSED_MESSAGES 10

class MotorController
{
public:
    enum class MotorMovingState
    {
        UNKNOWN,
        STOPPED,
        ACCELERATING,
        DECELERATING,
        FULL_SPEED,
        HOMING,
        CALIBRATING
    };

    MotorController(uint32_t id, SharedServices *shared_services, SpecificServices *specific_services);

    esp_err_t init();
    void handle_response(twai_message_t *msg);
    void handle_received_message(twai_message_t *twai_message_t);
    void handle_query_status_response(twai_message_t *msg);
    void handleQueryMotorPositionResponse(twai_message_t *msg);
    void handleSetPositionResponse(twai_message_t *msg);
    void handeSetHomeResponse(twai_message_t *msg);
    void handleSetWorkModeResponse(twai_message_t *msg);
    void handleSetCurrentResponse(twai_message_t *msg);
    void decodeMessage(twai_message_t *msg);

    esp_err_t set_target_position();
    esp_err_t query_position();
    esp_err_t query_status();

    uint32_t get_carry_value() const { return carry_value; }
    uint16_t get_encoder_value() const { return encoder_value; }
    MotorMovingState get_motor_moving_state() const { return motor_moving_state; }
    std::chrono::system_clock::time_point get_last_seen() const { return last_seen; }
    bool get_is_connected() const { return is_connected; }

    StateMachine::State get_state() const { return state_machine.get_state(); }
    void set_state(StateMachine::State new_state);

private:
    uint32_t canId;
    std::chrono::system_clock::time_point last_seen;
    bool is_connected = false;
    int error_counter = 0;
    MotorMovingState motor_moving_state = MotorMovingState::UNKNOWN;
    esp_err_t is_healthy();

    TWAIController *twai_controller;
    CommandMapper *command_mapper;

    QueueHandle_t outQ;
    QueueHandle_t inQ;

    StateMachine state_machine;
    TWAICommandFactory *command_factory;

    uint32_t carry_value;
    uint16_t encoder_value;
    std::string F5Status;

    //    static void vTask_handleInQ(void *pvParameters);
    static void vTask_queryPosition(void *pvParameters);
    static void vtask_sendPositon(void *pvParameters);
    static void vTask_queryStatus(void *pvParameters);
};

#endif // CANSERVO_H