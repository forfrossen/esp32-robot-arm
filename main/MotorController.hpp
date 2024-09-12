#ifndef MOTORCONTROLLER_H
#define MOTORCONTROLLER_H

#include "CommandMapper.hpp"
#include "Commands/TWAICommandFactory.hpp"
#include "ResponseHandlerRegistry.hpp"
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
    enum class State
    {
        IDLE,
        REQUESTED,
        MOVING,
        COMPLETED,
        ERROR
    };

    MotorController(uint32_t id, TWAIController *twai_controller, CommandMapper *command_mapper);

    void registerResponseHandler(uint8_t commandCode, std::function<void(uint8_t *, uint8_t)> handler);
    void handleResponse(uint8_t *data, uint8_t length);
    void handleReceivedMessage(twai_message_t *twai_message_t);
    void sendCommand(uint8_t *data, uint8_t length);
    void handleQueryStatusResponse(const uint8_t *data, uint8_t length);
    void handleQueryMotorPositionResponse(const uint8_t *data, uint8_t length);
    void handleSetPositionResponse(const uint8_t *data, uint8_t length);
    void handeSetHomeResponse(const uint8_t *data, uint8_t length);
    void handleSetWorkModeResponse(uint8_t *data, uint8_t length);
    void handleSetCurrentResponse(uint8_t *data, uint8_t length);
    void decodeMessage(const uint8_t *data, uint8_t length);
    esp_err_t set_target_position();
    esp_err_t query_position();

    uint32_t
    getCanId() const
    {
        return canId;
    }
    uint32_t getCarryValue() const { return CarryValue; }
    uint16_t getEncoderValue() const { return EncoderValue; }

    StateMachine::State getState() const { return stateMachine.getState(); }
    void setState(StateMachine::State newState);

private:
    uint32_t canId;
    TWAIController *twai_controller;
    QueueHandle_t outQ;
    QueueHandle_t inQ;
    CommandMapper *command_mapper;
    ResponseHandlerRegistry responseHandlerRegistry;
    uint32_t CarryValue;
    uint16_t EncoderValue;
    std::string F5Status;
    StateMachine stateMachine;
    int errorCounter = 0;

    TWAICommandFactory commandFactory;

    static void vTask_handleInQ(void *pvParameters);

    static void vTask_queryPosition(void *pvParameters);
    static void task_sendPositon(void *pvParameters);
};

#endif // CANSERVO_H