#include "Controller.hpp"
#include "Events.hpp"
// ESP_EVENT_DEFINE_BASE(MOTOR_EVENTS);

void MotorController::state_transition_event_handler(void *args, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    MotorController *instance = static_cast<MotorController *>(args);
    esp_err_t ret;

    ESP_LOGI(FUNCTION_NAME, "GOT MOTOR EVENT FOR MOTOR %lu, EVENT_BASE: %s, EVENT_ID: %lu", instance->canId, event_base, event_id);

    if (event_base != MOTOR_EVENTS)
    {
        return;
    }

    MotorContext::ReadyState new_state = static_cast<MotorContext::ReadyState>(reinterpret_cast<uintptr_t>(event_data));

    ESP_LOGI(FUNCTION_NAME, "New state: %d", new_state);
    // I (855) MotorController::MotorController: New MotorController-Object created with CAN ID: 3
    // I (865) MotorController::state_transition_event_handler: GOT MOTOR EVENT FOR MOTOR 3, EVENT_BASE: MOTOR_EVENTS, EVENT_ID: 0
    // I (875) MotorController::state_transition_event_handler: New state: 1073492120

    switch (new_state)
    {
    case MotorContext::ReadyState::MOTOR_INITIALIZED:
        ESP_LOGI("MotorEventLoopHandler", "Handling MOTOR_EVENT_INIT.");
        ret = instance->start_basic_tasks();
        if (ret != ESP_OK)
        {
            instance->context->transition_ready_state(MotorContext::MOTOR_ERROR);
        }
        break;

    case MotorContext::ReadyState::MOTOR_ERROR:
        ESP_LOGE("MotorEventLoopHandler", "Handling MOTOR_EVENT_ERROR.");
        xEventGroupClearBits(instance->motor_event_group, MOTOR_READY_BIT);
        xEventGroupSetBits(instance->motor_event_group, MOTOR_ERROR_BIT);
        // instance->stop_timed_tasks();
        break;

    case MotorContext::ReadyState::MOTOR_RECOVERING:
        ESP_LOGI("MotorEventLoopHandler", "Handling MOTOR_EVENT_RECOVERING.");
        ret = instance->start_timed_tasks();
        xEventGroupClearBits(instance->motor_event_group, MOTOR_ERROR_BIT);
        if (ret != ESP_OK)
        {
            instance->context->transition_ready_state(MotorContext::MOTOR_ERROR);
        }
        else
        {
            instance->context->transition_ready_state(MotorContext::MOTOR_READY);
        }
        break;

    case MotorContext::ReadyState::MOTOR_READY:
        ESP_LOGI("MotorEventLoopHandler", "Handling MOTOR_EVENT_READY.");
        xEventGroupSetBits(instance->motor_event_group, MOTOR_READY_BIT);
        break;
    default:
        break;
    }
}

void MotorController::vTask_query_position(void *pvParameters)
{

    MotorController *instance = static_cast<MotorController *>(pvParameters);
    vTaskDelay(8000 / portTICK_PERIOD_MS);

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
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    for (;;)
    {
        ESP_LOGI(FUNCTION_NAME, "New iteration of vTask_query_status");
        // esp_err_t ret;
        esp_err_t ret = instance->query_status();
        // ret = instance->query_position();
        if (ret != ESP_OK)
        {
            ESP_LOGE(FUNCTION_NAME, "Error enqueing query-status-command. Error counter: %d", local_error_counter);
            if (local_error_counter < 5)
            {
                local_error_counter++;
            }
        }

        vTaskDelay((4000 * (local_error_counter + 1)) / portTICK_PERIOD_MS);
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
                                                                 system_event_group(dependencies->event_groups->system_event_group),
                                                                 motor_event_group(dependencies->event_groups->motor_event_group),
                                                                 system_event_loop(dependencies->event_loops->system_event_loop),
                                                                 motor_event_loop(dependencies->event_loops->motor_event_loop),
                                                                 motor_mutex(dependencies->motor_mutex),
                                                                 command_lifecycle_registry(dependencies->command_lifecycle_registry),
                                                                 context(dependencies->motor_context),
                                                                 response_handler(dependencies->motor_response_handler)
{
    ESP_LOGI(FUNCTION_NAME, "New MotorController-Object created with CAN ID: %lu", canId);
    esp_err_t err = esp_event_handler_register_with(motor_event_loop, MOTOR_EVENTS, motor_event_id_t::STATE_TRANSITION_EVENT, &MotorController::state_transition_event_handler, this);

    if (err != ESP_OK)
    {
        ESP_LOGE(FUNCTION_NAME, "Failed to register event handler: %s", esp_err_to_name(err));
    }

    context->transition_ready_state(MotorContext::MOTOR_INITIALIZED);
}

MotorController::~MotorController()
{
    vTaskDelete(task_handle_query_status);
    vTaskDelete(task_handle_send_position);
    esp_event_loop_delete(motor_event_loop);
}

esp_err_t MotorController::start_basic_tasks()
{
    BaseType_t xRet;

    xRet = xTaskCreatePinnedToCore(&MotorController::vTask_query_status, "TASK_queryStatus", 1024 * 3, this, 1, &task_handle_query_status, tskNO_AFFINITY);
    ESP_RETURN_ON_FALSE(task_handle_query_status != NULL, ESP_FAIL, FUNCTION_NAME, "Failed to create TASK_queryStatus");
    ESP_LOGI(FUNCTION_NAME, "TASK_queryStatus created");
    return ESP_OK;
}

esp_err_t MotorController::start_timed_tasks()
{
    BaseType_t xReturned;
    esp_err_t ret;
    ret = set_working_current(1600);
    if (ret != ESP_OK)
    {
        ESP_LOGE(FUNCTION_NAME, "Error enqueueing setting working current command");
        context->transition_ready_state(MotorContext::MOTOR_ERROR);
    }

    ESP_LOGI(FUNCTION_NAME, "Initializing tasks");

    if (task_handle_query_position != nullptr)
    {
        vTaskDelete(task_handle_query_position);
    }
    xReturned = xTaskCreatePinnedToCore(&MotorController::vTask_query_position, "TASK_queryPosition", 1024 * 3, this, 3, &task_handle_query_position, tskNO_AFFINITY);
    ESP_RETURN_ON_FALSE(xReturned != NULL, ESP_FAIL, FUNCTION_NAME, "Failed to create TASK_queryPosition");
    ESP_LOGI(FUNCTION_NAME, "TASK_queryPosition created");
    if (task_handle_send_position != nullptr)
    {
        vTaskDelete(task_handle_send_position);
    }
    xReturned = xTaskCreatePinnedToCore(&MotorController::vtask_send_positon, "TASK_SendRandomTargetPositionCommands", 1024 * 3, this, 4, &task_handle_send_position, tskNO_AFFINITY);
    ESP_RETURN_ON_FALSE(xReturned != NULL, ESP_FAIL, FUNCTION_NAME, "Failed to create TASK_SendRandomTargetPositionCommands");
    ESP_LOGI(FUNCTION_NAME, "TASK_SendRandomTargetPositionCommands created");
    return ESP_OK;
}

void MotorController::stop_basic_tasks()
{
    vTaskDelete(task_handle_handle_inQ);
    vTaskDelete(task_handle_query_status);
}

void MotorController::stop_timed_tasks()
{
    vTaskDelete(task_handle_query_position);
    vTaskDelete(task_handle_send_position);
}

void MotorController::post_event(motor_event_id_t event)
{
    ESP_ERROR_CHECK(esp_event_post_to(motor_event_loop, MOTOR_EVENTS, event, NULL, 0, portMAX_DELAY));
}

esp_err_t MotorController::execute_query_command(std::function<TWAICommandBuilderBase<GenericCommandBuilder> *()> command_factory_method)
{
    ESP_LOGI(FUNCTION_NAME, "Executing query command");
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
    //  { return command_factory->query_motor_position_command(); });
    return execute_query_command([this]()
                                 { return command_factory->generate_new_generic_builder(CommandIds::READ_ENCODER_VALUE_CARRY); });
}

esp_err_t MotorController::query_status()
{
    //  { return command_factory->query_motor_status_command(); });
    return execute_query_command([this]()
                                 { return command_factory->generate_new_generic_builder(CommandIds::QUERY_MOTOR_STATUS); });
}

esp_err_t MotorController::set_working_current(uint16_t current_ma)
{
    return execute_query_command([this, current_ma]()
                                 {
                                     auto builder = command_factory->generate_new_generic_builder(CommandIds::SET_WORKING_CURRENT);
                                     builder->with(current_ma);
                                     return builder; });
}

esp_err_t MotorController::set_target_position()
{
    xSemaphoreTake(motor_mutex, portMAX_DELAY);
    int position = (esp_random() % 10) * STEPS_PER_REVOLUTION; // Random value between 0 and 99
    int speed = esp_random() % 1600;
    int acceleration = esp_random() % 255;
    bool absolute = esp_random() % 2;

    speed = 1000;
    acceleration = 10;
    absolute = false;

    // set_command_state(CommandStateMachine::CommandState::REQUESTED);
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
