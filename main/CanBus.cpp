#include "CanBus.hpp"

const char hex_asc_upper[] = "0123456789ABCDEF";

#define hex_asc_upper_lo(x) hex_asc_upper[((x) & 0x0F)]
#define hex_asc_upper_hi(x) hex_asc_upper[((x) & 0xF0) >> 4]

static inline void put_hex_byte(char *buf, __u8 byte)
{
  buf[0] = hex_asc_upper_hi(byte);
  buf[1] = hex_asc_upper_lo(byte);
}

static inline void _put_id(char *buf, int end_offset, canid_t id)
{
  /* build 3 (SFF) or 8 (EFF) digit CAN identifier */
  while (end_offset >= 0)
  {
    buf[end_offset--] = hex_asc_upper[id & 0xF];
    id >>= 4;
  }
}

#define put_sff_id(buf, id) _put_id(buf, 2, id)
#define put_eff_id(buf, id) _put_id(buf, 7, id)

void CanBus::printCANStatus()
{
  uint8_t errorFlag = mcp2515->getErrorFlags();

  ESP_LOGE(FUNCTION_NAME, "Error Flag: 0x%02X\n", errorFlag);

  if (errorFlag & MCP2515::EFLG_RX0OVR)
    ESP_LOGE(FUNCTION_NAME, "Receive Buffer 0 Overflow");
  if (errorFlag & MCP2515::EFLG_TXEP)
    ESP_LOGE(FUNCTION_NAME, "Transmit Error Passive");
  if (errorFlag & MCP2515::EFLG_TXBO)
    ESP_LOGE(FUNCTION_NAME, "Transmit Bus-Off");
  if (errorFlag & MCP2515::EFLG_RXEP)
    ESP_LOGE(FUNCTION_NAME, "Receive Error Passive");
  if (errorFlag & MCP2515::EFLG_TXWAR)
    ESP_LOGE(FUNCTION_NAME, "Transmit Warning");
  if (errorFlag & MCP2515::EFLG_RXWAR)
    ESP_LOGE(FUNCTION_NAME, "Receive Warning");
  if (errorFlag & MCP2515::EFLG_EWARN)
    ESP_LOGE(FUNCTION_NAME, "Error Warning");
}

CanBus::CanBus()
{
  ERROR error = setupSpi();
  if (error != ERROR_OK)
  {
    ESP_LOGE(FUNCTION_NAME, "Error setting up SPI");
  }
}

CanBus::~CanBus()
{
  _isConnected = false;
  delete mcp2515;
}

MCP2515 *CanBus::getMcp2515()
{
  return mcp2515;
}

void CanBus::begin()
{
  ESP_LOGI(FUNCTION_NAME, "BEGIN");

  mcp2515 = new MCP2515(&spi);
  mcp2515->reset();
  mcp2515->setConfigMode();
}

bool CanBus::isConnected()
{
  return _isConnected;
}

CanBus::ERROR CanBus::setupSpi()
{
  ESP_LOGI(FUNCTION_NAME, "setup Spi");
  esp_err_t ret;
  spi_bus_config_t buscfg = {
      .mosi_io_num = PIN_NUM_MOSI,
      .miso_io_num = PIN_NUM_MISO,
      .sclk_io_num = PIN_NUM_CLK,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .max_transfer_sz = 4096,
  };

  // Initialize the SPI bus
  ret = spi_bus_initialize(HSPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
  ESP_ERROR_CHECK(ret);

  spi_device_interface_config_t devcfg = {
      .mode = 0,                         // SPI mode 0
      .clock_speed_hz = 1 * 1000 * 1000, // 1 MHz
      .spics_io_num = PIN_NUM_CS,        // CS pin
      .queue_size = 7,                   // Queue size
      .pre_cb = NULL,
      .post_cb = NULL,
  };

  // Attach the MCP2515 to the SPI bus
  ret = spi_bus_add_device(HSPI_HOST, &devcfg, &spi);
  ESP_ERROR_CHECK(ret);

  return ERROR_OK;
}

CanBus::ERROR CanBus::connectCan()
{
  MCP2515::ERROR error = mcp2515->setBitrate(bitrate, canClock);
  if (error != MCP2515::ERROR_OK)
  {
    ESP_LOGI(FUNCTION_NAME, "setBitrate error: %d", error);
    return ERROR_MCP2515_INIT_BITRATE;
  }

  if (_loopback)
  {
    error = mcp2515->setLoopbackMode();
  }
  else if (_listenOnly)
  {
    error = mcp2515->setListenOnlyMode();
  }
  else
  {
    error = mcp2515->setNormalMode();
  }

  if (error != MCP2515::ERROR_OK)
  {
    ESP_LOGE(FUNCTION_NAME, "CAN BUS Shield could not be initialized!");
    return ERROR_MCP2515_INIT_SET_MODE;
  }

  ESP_LOGI(FUNCTION_NAME, "CAN BUS Shield init ok!\n");

  _isConnected = true;
  return ERROR_OK;
}

CanBus::ERROR CanBus::setupInterrupt()
{
  ESP_LOGI(FUNCTION_NAME, "Attaching interrupt");
  gpio_set_direction(PIN_NUM_INT, GPIO_MODE_INPUT);
  gpio_set_intr_type(PIN_NUM_INT, GPIO_INTR_NEGEDGE);
  gpio_install_isr_service(0);
  gpio_isr_handler_add(PIN_NUM_INT, irqHandler, this);
  return ERROR_OK;
}

CanBus::ERROR CanBus::setupQueues()
{
  ESP_LOGI(FUNCTION_NAME, "Creating Queue Buffers");
  outQ = xQueueCreate(10, sizeof(can_frame));

  if (outQ == NULL)
  {
    ESP_LOGE(FUNCTION_NAME, "Failed to create command queue");
    return ERROR_OUTQUEUE_NOT_INITIALIZED;
  }
  else
  {
    xTaskCreatePinnedToCore(vTask_handleSendQueue, "OutQueueTask", 3000, this, 5, &taskHandleOutQueue, 0);
    configASSERT(taskHandleOutQueue);
  }

  xTaskCreatePinnedToCore(vTask_handleReceiveQueue, "InQueuesTask", 3000, this, 5, &taskHandleCheckMessages, 0);
  configASSERT(taskHandleCheckMessages);

  return ERROR_OK;
}

void CanBus::registerInQueue(canid_t canId, QueueHandle_t inQ)
{
  inQs.emplace(canId, inQ);
}

CanBus::ERROR CanBus::disconnectCan()
{
  _isConnected = false;
  mcp2515->setConfigMode();
  return ERROR_OK;
}

uint16_t CanBus::getTimestamp()
{
  return xTaskGetTickCount() % TIMESTAMP_LIMIT;
}

void CanBus::irqHandler(void *arg)
{
  CanBus *canBus = static_cast<CanBus *>(arg);
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(canBus->taskHandleCheckMessages, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void CanBus::vTask_handleReceiveQueue(void *pvParameters)
{
  CanBus *canBus = static_cast<CanBus *>(pvParameters);

  while (1)
  {
    // Wait for a notification from the ISR
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    ESP_LOGI(FUNCTION_NAME, "Received interrupt notification");

    if (!canBus->isConnected())
    {
      ESP_LOGE(FUNCTION_NAME, "Process interrupt while not connected");
      continue;
    }

    uint8_t irq = canBus->mcp2515->getInterrupts();

    if (irq & MCP2515::CANINTF_ERRIF)
    {
      // reset RXnOVR errors
      canBus->mcp2515->clearRXnOVR();
    }

    if (irq & MCP2515::CANINTF_WAKIF || irq & MCP2515::CANINTF_MERRF)
    {
      ESP_LOGE(FUNCTION_NAME, "MCP_WAKIF or MCP_MERRF");
      canBus->mcp2515->clearInterrupts();
    }

    if (irq & MCP2515::CANINTF_ERRIF)
    {
      ESP_LOGE(FUNCTION_NAME, "ERRIF");
      canBus->mcp2515->clearMERR();
    }

    struct can_frame frame;
    MCP2515::ERROR result = canBus->mcp2515->readMessage(&frame);
    if (result == MCP2515::ERROR_NOMSG)
    {
      ESP_LOGE(FUNCTION_NAME, "No message received");
      canBus->mcp2515->clearInterrupts();
      continue;
    }

    if (result != MCP2515::ERROR_OK)
    {
      ESP_LOGE(FUNCTION_NAME, "Error reading message");
      continue;
    }

    if (!canBus->inQs.count(frame.can_id))
    {
      ESP_LOGE(FUNCTION_NAME, "No queue for can_id %lu", frame.can_id);
      continue;
    }
    else
    {
      ESP_LOGI(FUNCTION_NAME, "Queue found for can_id %lu", frame.can_id);
      if (xQueueSendToBack(canBus->inQs[frame.can_id], &frame, 0) == pdPASS)
      {
        ESP_LOGI(FUNCTION_NAME, "    ==> Received message enqueued successfully!");
        continue;
      }
      else
      {
        ESP_LOGE(FUNCTION_NAME, "    ==> Error enqueuing received message!");
        continue;
      }
    }
  }
}

uint8_t CanBus::calculateCRC(uint32_t id, const uint8_t *data, uint8_t length)
{
  uint8_t crc = id;
  for (uint8_t i = 0; i < length; i++)
  {
    crc += data[i];
  }
  return crc & 0xFF;
}

void CanBus::vTask_handleSendQueue(void *pvParameters)
{
  CanBus *canBus = static_cast<CanBus *>(pvParameters);

  while (1)
  {
    can_frame outQmsg;
    if (xQueueReceive(canBus->outQ, &outQmsg, portMAX_DELAY) == pdPASS)
    {
      canBus->sendCANMessage(outQmsg);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

bool CanBus::sendCANMessage(can_frame frame)
{
  ESP_LOGI(FUNCTION_NAME, "Sending message");
  if (frame.can_dlc <= 7)
  {
    uint8_t crc = calculateCRC(frame.can_id, frame.data, frame.can_dlc);
    frame.data[frame.can_dlc] = crc;
    frame.can_dlc += 1;
  }

  if (mcp2515->sendMessage(&frame) == MCP2515::ERROR_OK)
  {
    ESP_LOGI(FUNCTION_NAME, "Message sent");
    return true;
  }
  else
  {
    ESP_LOGE(FUNCTION_NAME, "Error sending message");
    printCANStatus(); // Print status and error flags

    ESP_LOGE(FUNCTION_NAME, "Data: ");
    for (int i = 0; i < frame.can_dlc; i++)
    {
      ESP_LOGE(FUNCTION_NAME, "%u", frame.data[i]);
    }
    return false;
  }
}

bool CanBus::listenForMessages(struct can_frame *frame)
{
  if (mcp2515->readMessage(frame) == MCP2515::ERROR_OK)
  {
    return true;
  }
  return false;
}
