#include "CANServo.hpp"
#include "../components/mcp2515/include/mcp2515.h"
#include "../components/mcp2515/include/can.h"
#include "utils.hpp"
#include "Commands/Command.hpp"
#include "Commands/Query/QueryMotorPositionCommand.hpp"

void CANServo::taskEntryPoint(void *pvParameters)
{
  CANServo *instance = static_cast<CANServo *>(pvParameters);
  instance->queryPosition(instance);
}

CANServo::CANServo(uint32_t id, CanBus *bus, CommandMapper *commandMapper) : canId(id), canBus(bus), commandMapper(commandMapper)
{
  const char *TAG = FUNCTION_NAME;
  ;
  ESP_LOGI(TAG, "New Servo42D_CAN object created with CAN ID: %lu", canId);

  xTaskCreatePinnedToCore(&CANServo::taskEntryPoint, "taskQueryMotorPosition", 512 * 2 * 4, this, 2, NULL, 0);
}

void CANServo::queryPosition(CANServo *servo)
{
  static const char *TAG = FUNCTION_NAME;
  vTaskDelay(2000 / portTICK_PERIOD_MS);

  for (;;)
  {
    ESP_LOGI(TAG, "New iteration of taskQueryMotorPosition");

    Command *queryMotorPosition = new QueryMotorPositionCommand(servo);
    queryMotorPosition->execute();
    delete queryMotorPosition;

    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

void CANServo::registerResponseHandler(uint8_t commandCode, std::function<void(uint8_t *, uint8_t)> handler)
{
  responseHandlerRegistry.registerHandler(commandCode, handler);
}

void CANServo::handleResponse(uint8_t *data, uint8_t length)
{
  responseHandlerRegistry.handleResponse(data, length);
}

void CANServo::handleReceivedMessage(can_frame *frame)
{
  static const char *TAG = FUNCTION_NAME;
  uint8_t code = frame->data[0];

  if (!code)
  {
    ESP_LOGE(TAG, "Error: code is empty!");
    return;
  }

  char commandName[50];
  commandMapper->getCommandNameFromCode(code, commandName);

  ESP_LOGI(TAG, "ID: %lu\t length: %u\tcode: %d\tcommandName: %s", canId, frame->can_dlc, frame->data[0], commandName);

  if (frame->can_id == canId)
  {
    decodeMessage(frame->data, frame->can_dlc);
  }
}

void CANServo::sendCommand(uint8_t *data, uint8_t length)
{
  static const char *TAG = FUNCTION_NAME;
  uint8_t code = data[0];

  if (!code)
  {
    ESP_LOGE(TAG, "Error: code is empty!");
    return;
  }

  char commandName[50];
  commandMapper->getCommandNameFromCode(code, commandName);

  ESP_LOGI(TAG, "ID: %lu\t length: %u\t code: %d\t commandName: %s", canId, length, data[0], commandName);

  if (canBus->sendCANMessage(canId, length, data))
  {
    ESP_LOGI(TAG, "    ==> Message sent successfully!");
  }
  else
  {
    ESP_LOGE(TAG, "    ==> Error sending message!");
  }
}

void CANServo::handleQueryStatusResponse(const uint8_t *data, uint8_t length)
{
  static const char *TAG = FUNCTION_NAME;
  uint8_t status = data[1];

  switch (status)
  {
  case 0:
    ESP_LOGI(TAG, "Abfrage fehlgeschlagen");
    break;
  case 1:
    ESP_LOGI(TAG, "Motor gestoppt");
    break;
  case 2:
    ESP_LOGI(TAG, "Motor beschleunigt");
    break;
  case 3:
    ESP_LOGI(TAG, "Motor verlangsamt");
    break;
  case 4:
    ESP_LOGI(TAG, "Motor volle Geschwindigkeit");
    break;
  case 5:
    ESP_LOGI(TAG, "Motor wird geparkt");
    break;
  case 6:
    ESP_LOGI(TAG, "Motor wird kalibriert");
    break;
  default:
    ESP_LOGI(TAG, "Unbekannter Status");
    break;
  }
}

void CANServo::handleQueryMotorPositionResponse(const uint8_t *data, uint8_t length)
{
  static const char *TAG = FUNCTION_NAME;
  if (length != 8 || data[0] != 0x30)
  {
    ESP_LOGE(TAG, "Invalid response length or code.");
    return;
  }

  CarryValue = (data[1] << 24) | (data[2] << 16) | (data[3] << 8) | data[4];
  EncoderValue = (data[5] << 8) | data[6];
  uint8_t crc = data[7];

  ESP_LOGI(TAG, "Carry value: %lu", CarryValue);
  ESP_LOGI(TAG, "Encoder value: %u", EncoderValue);
}

void CANServo::handleSetPositionResponse(const uint8_t *data, uint8_t length)
{
  static const char *TAG = FUNCTION_NAME;
  if (length != 3)
  {
    ESP_LOGE(TAG, "Invalid response length.");
    return;
  }
  if (data[0] != 0xF5)
  {
    ESP_LOGE(TAG, "Unexpected command code: %u", data[0]);
    return;
  }

  uint8_t status = data[1];
  uint8_t crc = data[2];

  std::string F5Status;
  switch (status)
  {
  case 0:
    F5Status = "Run failed";
    break;
  case 1:
    F5Status = "Run starting";
    break;
  case 2:
    F5Status = "Run complete";
    break;
  case 3:
    F5Status = "End limit stopped";
    break;
  default:
    F5Status = "Unknown status";
    break;
  }

  ESP_LOGI(TAG, "Set Position Response: %s", F5Status.c_str());
}

void CANServo::handeSetHomeResponse(const uint8_t *data, uint8_t length)
{
  static const char *TAG = FUNCTION_NAME;

  if (length != 3)
  {
    ESP_LOGE(TAG, "Invalid response length: %u", length);
    return;
  }

  if (data[0] != 0x90)
  {
    ESP_LOGE(TAG, "Unexpected command code: %u", data[0]);
    return;
  }

  uint8_t status = data[1];
  uint8_t crc = data[2];

  std::string statusMessage;
  switch (status)
  {
  case 0:
    statusMessage = "Set home failed.";
    break;
  case 1:
    statusMessage = "Set home in progress...";
    break;
  case 2:
    statusMessage = "Set home completed.";
    break;
  default:
    statusMessage = "Unknown status.";
    break;
  }

  ESP_LOGI(TAG, "Set Home Response: %s", statusMessage.c_str());
  ESP_LOGI(TAG, "CRC: %u", crc);
}

void CANServo::handleSetWorkModeResponse(uint8_t *data, uint8_t length)
{
  static const char *TAG = FUNCTION_NAME;
  if (data[1] == 1)
  {
    ESP_LOGI(TAG, "Set Work Mode: Success");
  }
  else
  {
    ESP_LOGE(TAG, "Set Work Mode: Failed");
  }
}

void CANServo::handleSetCurrentResponse(uint8_t *data, uint8_t length)
{
  static const char *TAG = FUNCTION_NAME;
  if (data[1] == 1)
  {
    ESP_LOGI(TAG, "Set Current: Success");
  }
  else
  {
    ESP_LOGE(TAG, "Set Current: Failed");
  }
}

void CANServo::decodeMessage(const uint8_t *data, uint8_t length)
{
  static const char *TAG = FUNCTION_NAME;

  ESP_LOGI(TAG, "CanId: %lu received message with code: %u", canId, data[0]);

  if (data[0] == 0x30)
  {
    handleQueryMotorPositionResponse(data, length);
  }
  else if (data[0] == 0xF5)
  {
    handleSetPositionResponse(data, length);
  }
  else if (data[0] == 0x31)
  {
    handleQueryStatusResponse(data, length);
  }
  else if (data[0] == 0x90)
  {
    handeSetHomeResponse(data, length);
  }
  else
  {
    ESP_LOGE(TAG, "unimplemented code: %d", data[0]);

    ESP_LOGI(TAG, "Raw code byte: %u", data[0]);

    for (int i = 0; i < length; i++)
    {
      ESP_LOGI(TAG, "Data[%d]: %d", i, data[i]);
    }
  }
}