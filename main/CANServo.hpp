#ifndef SERVO42D_CAN_H
#define SERVO42D_CAN_H

#include <map>
#include <functional>
#include <cstdint>
#include "CanBus.hpp"
#include "CommandMapper.hpp"
#include "../components/mcp2515/include/mcp2515.h"
#include "../components/mcp2515/include/can.h"
#include "ResponseHandlerRegistry.hpp"
#include "StateMachine.hpp"
#include "freertos/FreeRTOS.h"

#define MAX_PROCESSED_MESSAGES 10

class CANServo
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

  CANServo(uint32_t id, CanBus *canBus, CommandMapper *commandMapper);

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

  uint32_t getCanId() const { return canId; }
  uint32_t getCarryValue() const { return CarryValue; }
  uint16_t getEncoderValue() const { return EncoderValue; }

  StateMachine::State getState() const { return stateMachine.getState(); }
  void setState(StateMachine::State newState);

private:
  uint32_t canId;
  CanBus *canBus;
  QueueHandle_t outQ;
  QueueHandle_t inQ;
  CommandMapper *commandMapper;
  ResponseHandlerRegistry responseHandlerRegistry;
  uint32_t CarryValue;
  uint16_t EncoderValue;
  std::string F5Status;
  StateMachine stateMachine;

  static void vTask_handleInQ(void *pvParameters);

  static void vTask_queryPosition(void *pvParameters);
  static void task_sendPositon(void *pvParameters);
};

#endif // CANSERVO_H