#ifndef MOTORCONTROLLER_H
#define MOTORCONTROLLER_H

#include "CommandMapper.hpp"
#include "Commands/TWAICommandFactory.hpp"
#include "StateMachine.hpp"
#include "TWAIController.hpp"
#include "freertos/FreeRTOS.h"
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

    MotorController(uint32_t id, TWAIController *twai_controller, CommandMapper *command_mapper);
    MotorMovingState motorMovingState = MotorMovingState::UNKNOWN;
    void registerResponseHandler(uint8_t commandCode, std::function<void(uint8_t *, uint8_t)> handler);

    void handleResponse(twai_message_t *msg);
    void handleReceivedMessage(twai_message_t *twai_message_t);
    void handleQueryStatusResponse(twai_message_t *msg);
    void handleQueryMotorPositionResponse(twai_message_t *msg);
    void handleSetPositionResponse(twai_message_t *msg);
    void handeSetHomeResponse(twai_message_t *msg);
    void handleSetWorkModeResponse(twai_message_t *msg);
    void handleSetCurrentResponse(twai_message_t *msg);
    void decodeMessage(twai_message_t *msg);
    esp_err_t set_target_position();
    esp_err_t query_position();

    uint32_t
    getCanId() const
    {
        return canId;
    }
    uint32_t getCarryValue() const { return carry_value; }
    uint16_t getEncoderValue() const { return encoder_value; }

    StateMachine::State getState() const { return stateMachine.getState(); }
    void setState(StateMachine::State newState);

private:
    uint32_t canId;
    TWAIController *twai_controller;
    QueueHandle_t outQ;
    QueueHandle_t inQ;

    CommandMapper *command_mapper;
    StateMachine stateMachine;
    TWAICommandFactory commandFactory;

    uint32_t carry_value;
    uint16_t encoder_value;
    std::string F5Status;

    //    static void vTask_handleInQ(void *pvParameters);
    static void vTask_queryPosition(void *pvParameters);
    static void task_sendPositon(void *pvParameters);
};

#endif // CANSERVO_H