#include "MotorController.hpp"

// Define the event base for motor controller events
ESP_EVENT_DEFINE_BASE(MOTOR_CONTROLLER_EVENT);

void MotorController::motor_controller_event_handler(void *handler_args, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    MotorController *instance = static_cast<MotorController *>(handler_args);

    if (event_base != MOTOR_CONTROLLER_EVENT)
    {
        return;
    }

    switch (event_id)
    {
    case MOTOR_EVENT_INIT:
        ESP_LOGI("MotorEventLoopHandler", "Handling MOTOR_EVENT_INIT.");
        instance->init_event_loop();
        break;

    case MOTOR_EVENT_ERROR:
        ESP_LOGE("MotorEventLoopHandler", "Handling MOTOR_EVENT_ERROR.");

        break;

    case MOTOR_EVENT_RECOVERING:
        ESP_LOGI("MotorEventLoopHandler", "Handling MOTOR_EVENT_RECOVERING.");

        break;

    default:
        break;
    }
}

void MotorController::vTask_handle_unhealthy(void *pvParameters)
{
    MotorController *instance = static_cast<MotorController *>(pvParameters);
    for (;;)
    {
        EventBits_t motor_bits = xEventGroupWaitBits(instance->motor_event_group, MOTOR_RECOVERING, pdFALSE, pdFALSE, portMAX_DELAY);

        if (motor_bits & MOTOR_RECOVERING)
        {
            ESP_LOGE(FUNCTION_NAME, "Motor in recovery state.");
            instance->init_tasks();
            instance->error_counter = 0;
            instance->transition_motor_state(MotorController::MOTOR_READY);
        }

        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void MotorController::vTask_query_position(void *pvParameters)
{

    MotorController *instance = static_cast<MotorController *>(pvParameters);
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    for (;;)
    {
        EventBits_t motor_bits = xEventGroupWaitBits(instance->motor_event_group, MOTOR_READY, pdFALSE, pdFALSE, portMAX_DELAY);
        if (motor_bits & MOTOR_READY)
        {
            ESP_LOGI(FUNCTION_NAME, "New iteration of taskQueryMotorPosition");
            esp_err_t ret = instance->query_position();
            if (ret != ESP_OK)
            {
                ESP_LOGE(FUNCTION_NAME, "Error enqueing query-status-command");
                if (instance->error_counter < 5)
                {
                    instance->error_counter++;
                }
            }
        }

        // ESP_LOGE(FUNCTION_NAME, "Motor is not connected. Stopping vTask_query_position");
        // vTaskDelete(NULL);

        // if (instance->is_healthy() != ESP_OK && instance->error_counter > 5)
        // {
        //     ESP_LOGE(FUNCTION_NAME, "Motor is not healthy. Stopping vTask_query_position");
        //     break;
        // }

        vTaskDelay(4000 / portTICK_PERIOD_MS);
    }
}

void MotorController::vTask_query_status(void *pvParameters)
{
    MotorController *instance = static_cast<MotorController *>(pvParameters);
    int local_error_counter = 0;
    for (;;)
    {
        ESP_LOGI(FUNCTION_NAME, "New iteration of vTask_query_status");

        esp_err_t ret = instance->query_status();
        if (ret != ESP_OK)
        {
            ESP_LOGE(FUNCTION_NAME, "Error enqueing query-status-command. Error counter: %d", local_error_counter);
            if (local_error_counter < 5)
            {
                local_error_counter++;
            }
        }

        vTaskDelay((1000 * (local_error_counter + 1)) / portTICK_PERIOD_MS);
    }
}

void MotorController::vTask_handleInQ(void *vParameters)
{
    MotorController *instance = static_cast<MotorController *>(vParameters);
    for (;;)
    {
        twai_message_t twai_message_t;
        xQueueReceive(instance->inQ, &twai_message_t, portMAX_DELAY);
        ESP_LOGI(FUNCTION_NAME, "Received message from inQ with ID: %lu", twai_message_t.identifier);
        instance->handle_received_message(&twai_message_t);
    }
}

void MotorController::vtask_send_positon(void *pvParameters)
{
    MotorController *instance = static_cast<MotorController *>(pvParameters);
    vTaskDelay(8000 / portTICK_PERIOD_MS);
    for (;;)
    {

        EventBits_t motor_bits = xEventGroupWaitBits(instance->motor_event_group, MOTOR_READY, pdFALSE, pdFALSE, portMAX_DELAY);
        if (motor_bits & MOTOR_READY)
        {

            ESP_LOGI(FUNCTION_NAME, "New iteration of taskSendRandomTargetPositionCommands");
            instance->set_target_position();
        }

        vTaskDelay(6000 / portTICK_PERIOD_MS);
    }
}

MotorController::MotorController(
    std::shared_ptr<MotorControllerDependencies> dependencies) : canId(dependencies->id),
                                                                 command_factory(dependencies->command_factory),
                                                                 inQ(dependencies->inQ),
                                                                 outQ(dependencies->outQ),
                                                                 command_mapper(dependencies->command_mapper),
                                                                 command_lifecycle_registry(dependencies->command_lifecycle_registry)
{
    BaseType_t xRet;
    ESP_LOGI(FUNCTION_NAME, "New MotorController-Object created with CAN ID: %lu", canId);

    motor_event_group = xEventGroupCreate();

    if (motor_event_group == NULL)
    {
        ESP_LOGE(FUNCTION_NAME, "Error creating event group");
    }

    if (inQ == nullptr)
    {
        ESP_LOGE(FUNCTION_NAME, "inQ is nullptr");
    }
    if (outQ == nullptr)
    {
        ESP_LOGE(FUNCTION_NAME, "outQ is nullptr");
    }

    init_event_loop();

    transition_motor_state(MotorState::MOTOR_INIT);

    xRet = xTaskCreatePinnedToCore(&MotorController::vTask_handleInQ, "TASK_handleInQ", 1024 * 3, this, 2, &task_handle_handle_inQ, 1);
    if (xRet != pdPASS)
    {
        ESP_LOGE(FUNCTION_NAME, "Failed to create TASK_handleInQ");
    }
    configASSERT(task_handle_handle_inQ);

    xRet = xTaskCreatePinnedToCore(&MotorController::vTask_query_status, "TASK_queryPosition", 1024 * 3, this, 1, &task_handle_query_status, 1);
    if (xRet != pdPASS)
    {
        ESP_LOGE(FUNCTION_NAME, "Failed to create TASK_queryPosition");
    }
    configASSERT(task_handle_query_status);
}

MotorController::~MotorController()
{
    vTaskDelete(task_handle_handle_inQ);
    vTaskDelete(task_handle_query_status);
    vTaskDelete(task_handle_send_position);
    vTaskDelete(task_handle_handle_unhealthy);
    esp_event_loop_delete(motor_controller_event_loop_handle);
}

void MotorController::init_event_loop()
{
    esp_event_loop_args_t loop_args = {
        .queue_size = 10,
        .task_name = "motor_controller_event_loop_task", // Name der Event Loop Task
        .task_priority = uxTaskPriorityGet(NULL),
        .task_stack_size = 4048,
        .task_core_id = tskNO_AFFINITY};
    ESP_ERROR_CHECK(esp_event_loop_create(&loop_args, &motor_controller_event_loop_handle));

    ESP_ERROR_CHECK(esp_event_handler_register_with(motor_controller_event_loop_handle, MOTOR_CONTROLLER_EVENT, ESP_EVENT_ANY_ID, &MotorController::motor_controller_event_handler, this));
}

esp_err_t MotorController::init_tasks()
{
    xSemaphoreTake(motor_mutex, portMAX_DELAY);
    esp_err_t ret = ESP_OK;
    BaseType_t xReturned;

    ESP_LOGI(FUNCTION_NAME, "Initializing tasks");

    // xRet = xTaskCreatePinnedToCore(&MotorController::vTask_handle_unhealthy, "TASK_handleUnhealthy", 1024 * 3, this, 1, &task_handle_handle_unhealthy, 1);
    // if (xRet != pdPASS)
    // {
    //     ESP_LOGE(FUNCTION_NAME, "Failed to create TASK_handleUnhealthy");
    // }
    // configASSERT(task_handle_handle_unhealthy);

    if (task_handle_query_position != nullptr)
    {
        vTaskDelete(task_handle_query_position);
    }
    xReturned = xTaskCreatePinnedToCore(&MotorController::vTask_query_position, "TASK_queryPosition", 1024 * 3, this, 3, &task_handle_query_position, 1);
    if (xReturned != pdPASS)
    {
        ESP_LOGE(FUNCTION_NAME, "Failed to create task_queryPosition");
        ret = ESP_FAIL;
    }
    configASSERT(vTask_query_position);

    if (task_handle_send_position != nullptr)
    {
        vTaskDelete(task_handle_send_position);
    }
    xReturned = xTaskCreatePinnedToCore(&MotorController::vtask_send_positon, "TASK_SendRandomTargetPositionCommands", 1024 * 3, this, 4, &task_handle_send_position, 1);
    if (xReturned != pdPASS)
    {
        ESP_LOGE(FUNCTION_NAME, "Failed to create task_sendRandomTargetPositionCommands");
        ret = ESP_FAIL;
    }
    configASSERT(vtask_send_positon);

    xSemaphoreGive(motor_mutex);
    return ret;
}

MotorController::MotorState MotorController::get_motor_state()
{
    xSemaphoreTake(motor_mutex, portMAX_DELAY);
    MotorState state = fsm_motor_state;
    xSemaphoreGive(motor_mutex);
    return state;
}

// Post events to the event loop
void MotorController::post_event(MotorEvent event)
{
    esp_event_post_to(motor_controller_event_loop_handle, MOTOR_CONTROLLER_EVENT, event, NULL, 0, portMAX_DELAY);
}

void MotorController::transition_motor_state(MotorController::MotorState newState)
{
    xSemaphoreTake(motor_mutex, portMAX_DELAY);
    fsm_motor_state = newState;
    switch (newState)
    {
    case MOTOR_INIT:
        ESP_LOGI("Motor", "Motor is initializing.");
        post_event(MotorEvent::MOTOR_EVENT_INIT);
        break;
    case MOTOR_READY:
        ESP_LOGI("Motor", "Motor is now ready.");
        post_event(MotorEvent::MOTOR_EVENT_READY);
        break;
    case MOTOR_ERROR:
        ESP_LOGE("Motor", "Motor encountered an error.");
        post_event(MotorEvent::MOTOR_EVENT_ERROR);
        break;
    case MOTOR_RECOVERING:
        ESP_LOGI("Motor", "Motor is recovering from error.");
        post_event(MotorEvent::MOTOR_EVENT_RECOVERING);
        break;
    default:
        break;
    }
    xSemaphoreGive(motor_mutex);
}

esp_err_t MotorController::query_position()
{
    xSemaphoreTake(motor_mutex, portMAX_DELAY);
    auto cmd = command_factory->query_motor_position_command();
    esp_err_t ret = cmd->build_and_send();
    if (ret != ESP_OK)
    {
        ESP_LOGE(FUNCTION_NAME, "Error enqueing query-status-command");
        transition_motor_state(MotorState::MOTOR_ERROR);
    }
    delete cmd;
    xSemaphoreGive(motor_mutex);
    return ret;
}

esp_err_t MotorController::query_status()
{
    xSemaphoreTake(motor_mutex, portMAX_DELAY);
    auto cmd = command_factory->query_motor_status_command();
    esp_err_t ret = cmd->build_and_send();
    if (ret != ESP_OK)
    {
        ESP_LOGE(FUNCTION_NAME, "Error enqueing query-status-command");
        transition_motor_state(MotorState::MOTOR_ERROR);
    }
    delete cmd;
    xSemaphoreGive(motor_mutex);
    return ret;
}

esp_err_t MotorController::set_target_position()
{
    xSemaphoreTake(motor_mutex, portMAX_DELAY);
    int position = (esp_random() % 10) * STEPS_PER_REVOLUTION; // Random value between 0 and 99
    int speed = esp_random() % 1600;
    int acceleration = esp_random() % 255;
    bool absolute = esp_random() % 2;

    speed = 250;
    acceleration = 255;
    absolute = false;

    set_state(MotorControllerFSM::State::REQUESTED);
    auto cmd = command_factory->create_set_target_position_command();
    esp_err_t ret = cmd->set_position(position).set_speed(speed).set_acceleration(acceleration).set_absolute(absolute).build_and_send();
    if (ret != ESP_OK)
    {
        ESP_LOGE(FUNCTION_NAME, "Error enqueing query-status-command");
        transition_motor_state(MotorState::MOTOR_ERROR);
    }
    delete cmd;
    xSemaphoreGive(motor_mutex);
    return ret;
}

void MotorController::set_state(MotorControllerFSM::State newState)
{
    fsm_moving.set_state(newState);
}

void MotorController::handle_received_message(twai_message_t *msg)
{

    if (!msg->data[0])
    {
        ESP_LOGE(FUNCTION_NAME, "Error: code is empty!");
        return;
    }

    char command_name[50];
    command_mapper->get_command_name_from_code(msg->data[0], command_name);

    ESP_LOGI(FUNCTION_NAME, "ID: %lu\t length: %u\tcode: %d\tcommandName: %s", canId, msg->data_length_code, msg->data[0], command_name);

    uint8_t *data = msg->data;
    uint8_t length = msg->data_length_code;

    switch (data[0])
    {
    case 0x30:
        handleQueryMotorPositionResponse(msg);
        break;
        break;
    case 0xF5:
    case 0xF4:
        handleSetPositionResponse(msg);
        break;
    case 0xF1:
        handle_query_status_response(msg);
        break;
    case 0x90:
        handeSetHomeResponse(msg);
        break;
    default:
        ESP_LOGE(FUNCTION_NAME, "unimplemented code: %02X", data[0]);
        ESP_LOGI(FUNCTION_NAME, "Raw code byte: %u", data[0]);
        for (int i = 0; i < length; i++)
        {
            ESP_LOGI(FUNCTION_NAME, "Data[%d]: %d", i, data[i]);
        }
    }
}

void MotorController::handle_query_status_response(twai_message_t *msg)
{
    uint8_t status = msg->data[1];
    if (status > 1 && status < 7)
    {
        if (get_motor_state() == MotorState::MOTOR_ERROR)
        {
            transition_motor_state(MotorState::MOTOR_RECOVERING);
        }
    }
    else
    {
        transition_motor_state(MotorState::MOTOR_ERROR);
    }

    switch (status)
    {
    case 0:
        ESP_LOGE(FUNCTION_NAME, "Abfrage fehlgeschlagen");
        motor_operating_state = OperatingState::UNKNOWN;
        break;
    case 1:
        ESP_LOGI(FUNCTION_NAME, "Motor gestoppt");
        motor_operating_state = OperatingState::STOPPED;
        break;
    case 2:
        ESP_LOGI(FUNCTION_NAME, "Motor beschleunigt");

        motor_operating_state = OperatingState::ACCELERATING;
        break;
    case 3:
        ESP_LOGI(FUNCTION_NAME, "Motor verlangsamt");
        motor_operating_state = OperatingState::DECELERATING;
        break;
    case 4:
        ESP_LOGI(FUNCTION_NAME, "Motor volle Geschwindigkeit");
        motor_operating_state = OperatingState::FULL_SPEED;
        break;
    case 5:
        ESP_LOGI(FUNCTION_NAME, "Motor fährt nach Hause");
        motor_operating_state = OperatingState::HOMING;
        break;
    case 6:
        ESP_LOGI(FUNCTION_NAME, "Motor wird kalibriert");
        motor_operating_state = OperatingState::CALIBRATING;
        break;
    default:
        ESP_LOGI(FUNCTION_NAME, "Unbekannter Status, Resonse: %02X", status);
        motor_operating_state = OperatingState::UNKNOWN;
        break;
    }
}

void MotorController::handleQueryMotorPositionResponse(twai_message_t *msg)
{

    if (msg->data_length_code != 8 || msg->data[0] != 0x30)
    {
        ESP_LOGE(FUNCTION_NAME, "Invalid response length or code.");
        return;
    }

    carry_value = (msg->data[1] << 24) | (msg->data[2] << 16) | (msg->data[3] << 8) | msg->data[4];
    encoder_value = (msg->data[5] << 8) | msg->data[6];

    ESP_LOGI(FUNCTION_NAME, "Carry value: %lu", carry_value);
    ESP_LOGI(FUNCTION_NAME, "Encoder value: %u", encoder_value);
}

void MotorController::handleSetPositionResponse(twai_message_t *msg)
{

    if (msg->data_length_code != 3)
    {
        ESP_LOGE(FUNCTION_NAME, "Invalid response length.");
        return;
    }
    if (msg->data[0] != 0xF5 && msg->data[0] != 0xF4)
    {
        ESP_LOGE(FUNCTION_NAME, "Unexpected command code: %u", msg->data[0]);
        return;
    }

    uint8_t status = msg->data[1];

    std::string F5Status;
    switch (status)
    {
    case 0:
        F5Status = "Run failed";
        fsm_moving.set_state(MotorControllerFSM::State::ERROR);
        break;
    case 1:
        F5Status = "Run starting";
        fsm_moving.set_state(MotorControllerFSM::State::MOVING);
        break;
    case 2:
        F5Status = "Run complete";
        fsm_moving.set_state(MotorControllerFSM::State::COMPLETED);
        break;
    case 3:
        F5Status = "End limit stopped";
        fsm_moving.set_state(MotorControllerFSM::State::COMPLETED);
        break;
    default:
        F5Status = "Unknown status";
        fsm_moving.set_state(MotorControllerFSM::State::ERROR);
        break;
    }

    ESP_LOGI(FUNCTION_NAME, "Set Position Response: %s", F5Status.c_str());
}

void MotorController::handeSetHomeResponse(twai_message_t *msg)
{

    if (msg->data_length_code != 3)
    {
        ESP_LOGE(FUNCTION_NAME, "Invalid response length: %u", msg->data_length_code);
        return;
    }

    if (msg->data[0] != 0x90)
    {
        ESP_LOGE(FUNCTION_NAME, "Unexpected command code: %u", msg->data[0]);
        return;
    }

    uint8_t status = msg->data[1];
    uint8_t crc = msg->data[2];

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

    ESP_LOGI(FUNCTION_NAME, "Set Home Response: %s", statusMessage.c_str());
    ESP_LOGI(FUNCTION_NAME, "CRC: %u", crc);
}

void MotorController::handleSetWorkModeResponse(twai_message_t *msg)
{

    if (msg->data[1] == 1)
    {
        ESP_LOGI(FUNCTION_NAME, "Set Work Mode: Success");
    }
    else
    {
        ESP_LOGE(FUNCTION_NAME, "Set Work Mode: Failed");
    }
}

void MotorController::handleSetCurrentResponse(twai_message_t *msg)
{

    if (msg->data[1] == 1)
    {
        ESP_LOGI(FUNCTION_NAME, "Set Current: Success");
    }
    else
    {
        ESP_LOGE(FUNCTION_NAME, "Set Current: Failed");
    }
}
