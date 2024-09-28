#include "MotorController.hpp"

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
        EventBits_t motor_bits = xEventGroupWaitBits(instance->motor_event_group, MotorContext::MOTOR_RECOVERING, pdFALSE, pdFALSE, portMAX_DELAY);

        if (motor_bits & MotorContext::MOTOR_RECOVERING)
        {
            ESP_LOGE(FUNCTION_NAME, "Motor in recovery state.");
            instance->init_tasks();
            instance->error_counter = 0;
            instance->context->transition_ready_state(MotorContext::MOTOR_READY);
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
        EventBits_t motor_bits = xEventGroupWaitBits(instance->motor_event_group, MotorContext::MOTOR_READY, pdFALSE, pdFALSE, portMAX_DELAY);
        if (motor_bits & MotorContext::MOTOR_READY)
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
        instance->response_handler->handle_received_message(&twai_message_t);
    }
}

void MotorController::vtask_send_positon(void *pvParameters)
{
    MotorController *instance = static_cast<MotorController *>(pvParameters);
    vTaskDelay(8000 / portTICK_PERIOD_MS);
    for (;;)
    {

        EventBits_t motor_bits = xEventGroupWaitBits(instance->motor_event_group, MOTOR_READY_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
        if (motor_bits & MOTOR_READY_BIT)
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
                                                                 command_lifecycle_registry(dependencies->command_lifecycle_registry),
                                                                 context(dependencies->motor_context),
                                                                 response_handler(dependencies->motor_response_handler)
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

    context->transition_ready_state(MotorContext::MOTOR_INIT);

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

void MotorController::post_event(MotorEvent event)
{
    esp_event_post_to(motor_controller_event_loop_handle, MOTOR_CONTROLLER_EVENT, event, NULL, 0, portMAX_DELAY);
}

esp_err_t MotorController::execute_query_command(std::function<TWAICommandBuilderBase<GenericCommandBuilder> *()> command_factory_method)
{
    xSemaphoreTake(motor_mutex, portMAX_DELAY);
    auto cmd = command_factory_method();
    esp_err_t ret = cmd->build_and_send();
    if (ret != ESP_OK)
    {
        ESP_LOGE(FUNCTION_NAME, "Error enqueing query command");
        context->transition_ready_state(MotorContext::MOTOR_ERROR);
    }
    delete cmd;
    xSemaphoreGive(motor_mutex);
    return ret;
}

esp_err_t MotorController::query_position()
{
    return execute_query_command([this]()
                                 { return command_factory->query_motor_position_command(); });
}

esp_err_t MotorController::query_status()
{
    return execute_query_command([this]()
                                 { return command_factory->query_motor_status_command(); });
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

    set_command_state(CommandStateMachine::CommandState::REQUESTED);
    auto cmd = command_factory->create_set_target_position_command();
    esp_err_t ret = cmd->set_position(position).set_speed(speed).set_acceleration(acceleration).set_absolute(absolute).build_and_send();
    if (ret != ESP_OK)
    {
        ESP_LOGE(FUNCTION_NAME, "Error enqueing query-status-command");
        context->transition_ready_state(MotorContext::MOTOR_ERROR);
    }
    delete cmd;
    xSemaphoreGive(motor_mutex);
    return ret;
}
