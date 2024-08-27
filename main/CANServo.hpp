#ifndef SERVO42D_CAN_H
#define SERVO42D_CAN_H

#include <map>
#include <functional>
#include <cstdint>
#include "CanBus.h"
#include "CommandMapper.hpp"
#include "../components/mcp2515/include/mcp2515.h"
#include "../components/mcp2515/include/can.h"
#include "ResponseHandlerRegistry.hpp"

#define MAX_PROCESSED_MESSAGES 10

class CANServo
{
public:
  CANServo(uint32_t id, CanBus *bus, CommandMapper *commandMapper);

  void registerResponseHandler(uint8_t commandCode, std::function<void(uint8_t *, uint8_t)> handler);
  void handleResponse(uint8_t *data, uint8_t length);
  void handleReceivedMessage(can_frame *frame);
  void sendCommand(uint8_t *data, uint8_t length);
  void handleQueryStatusResponse(const uint8_t *data, uint8_t length);
  void handleQueryMotorPositionResponse(const uint8_t *data, uint8_t length);
  void handleSetPositionResponse(const uint8_t *data, uint8_t length);
  void handeSetHomeResponse(const uint8_t *data, uint8_t length);
  void handleSetWorkModeResponse(uint8_t *data, uint8_t length);
  void handleSetCurrentResponse(uint8_t *data, uint8_t length);
  void decodeMessage(const uint8_t *data, uint8_t length);
  // static void taskFunctionWrapper(void *pvParameters);
  void taskCheckForMessages();
  void queryPosition(CANServo *servo);

  uint32_t getCanId() const { return canId; }
  uint32_t getCarryValue() const { return CarryValue; }
  uint16_t getEncoderValue() const { return EncoderValue; }

private:
  uint32_t canId;
  CanBus *canBus;
  CommandMapper *commandMapper;
  ResponseHandlerRegistry responseHandlerRegistry;
  uint32_t CarryValue;
  uint16_t EncoderValue;
  std::string F5Status;
  static void taskEntryPoint(void *pvParameters);
};

#endif // CANSERVO_H