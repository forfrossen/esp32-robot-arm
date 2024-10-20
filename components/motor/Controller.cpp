#include "Controller.hpp"
#include "Events.hpp"

ESP_EVENT_DEFINE_BASE(MOTOR_EVENTS);

int24_t calculate_steps_for_angle(int24_t angleDegrees)
{

    const double fullRotation = 360.0;
    double steps = (STEPS_PER_REVOLUTION / fullRotation) * angleDegrees;
    int24_t stepCount = static_cast<int24_t>(std::round(steps));

    return stepCount;
}

void MotorController::state_transition_event_handler(void *handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    MotorController *instance = static_cast<MotorController *>(handler_arg);
    RETURN_VOID_IF(instance == nullptr);

    esp_err_t ret;
    ESP_LOGI(FUNCTION_NAME, "GOT MOTOR EVENT FOR MOTOR %lu, EVENT_BASE: %s, EVENT_ID: %lu", instance->canId, event_base, event_id);
    RETURN_VOID_IF(event_base != MOTOR_EVENTS);
    RETURN_VOID_IF(event_id != STATE_TRANSITION_EVENT);

    MotorContext::ReadyState *new_state = (MotorContext::ReadyState *)event_data;
    const char *state_name = magic_enum::enum_name(*new_state).data();
    state_name == nullptr && (state_name = "UNKNOWN");

    ESP_LOGI(FUNCTION_NAME, "TRANSITION_STATE: %s", state_name);

    switch (*new_state)
    {
    case MotorContext::ReadyState::MOTOR_RECOVERING:
        ESP_LOGI("MotorEventLoopHandler", "Handling MOTOR_EVENT_RECOVERING.");
    case MotorContext::ReadyState::MOTOR_INITIALIZED:
        ESP_LOGI("MotorEventLoopHandler", "Handling MOTOR_EVENT_INIT.");
        ret = instance->start_basic_tasks();
        if (ret != ESP_OK)
        {
            ESP_LOGE("MotorEventLoopHandler", "Error starting basic tasks. Error: %s", esp_err_to_name(ret));
            instance->context->transition_ready_state(MotorContext::ReadyState::MOTOR_ERROR);
        }
        break;

    case MotorContext::ReadyState::MOTOR_ERROR:
        ESP_LOGE("MotorEventLoopHandler", "Handling MOTOR_EVENT_ERROR.");
        xEventGroupClearBits(instance->motor_event_group, MOTOR_READY_BIT);
        xEventGroupSetBits(instance->motor_event_group, MOTOR_ERROR_BIT);
        instance->context->transition_ready_state(MotorContext::ReadyState::MOTOR_RECOVERING);
        // instance->stop_timed_tasks();
        break;

    case MotorContext::ReadyState::MOTOR_READY:
        ESP_LOGI("MotorEventLoopHandler", "Handling MOTOR_EVENT_READY.");
        xEventGroupClearBits(instance->motor_event_group, MOTOR_ERROR_BIT);
        xEventGroupSetBits(instance->motor_event_group, MOTOR_READY_BIT);
        ret = instance->start_timed_tasks();
        ret == ESP_OK ? instance->context->transition_ready_state(MotorContext::ReadyState::MOTOR_READY)
                      : instance->context->transition_ready_state(MotorContext::ReadyState::MOTOR_ERROR);
        break;
    default:
        break;
    }
}

void MotorController::vTask_query_position(void *pvParameters)
{
    MotorController *instance = static_cast<MotorController *>(pvParameters);
    for (;;)
    {
        ESP_LOGI(FUNCTION_NAME, "New iteration of taskQueryMotorPosition");
        if (xEventGroupWaitBits(instance->motor_event_group, MOTOR_READY_BIT, pdFALSE, pdFALSE, portMAX_DELAY) & MOTOR_READY_BIT)
        {
            ESP_LOGI(FUNCTION_NAME, "New iteration of taskQueryMotorPosition");
            instance->query_position();
        }

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
            local_error_counter < 5 && local_error_counter++;
        }
        else
        {
            local_error_counter = 0;
            ESP_LOGI(FUNCTION_NAME, "Query status command enqueued successfully");
        }

        vTaskDelay((4000 * (local_error_counter + 1)) / portTICK_PERIOD_MS);
    }
}

void MotorController::vtask_send_positon(void *pvParameters)
{
    MotorController *instance = static_cast<MotorController *>(pvParameters);
    for (;;)
    {
        if (xEventGroupWaitBits(instance->motor_event_group, MOTOR_READY_BIT, pdFALSE, pdFALSE, portMAX_DELAY) & MOTOR_READY_BIT)
        {
            ESP_LOGI(FUNCTION_NAME, "New iteration of taskSendRandomTargetPositionCommands");
            instance->set_target_position();
        }

        vTaskDelay(6000 / portTICK_PERIOD_MS);
    }
}

MotorController::MotorController(
    std::shared_ptr<MotorControllerDependencies> dependencies) : canId(dependencies->id),
                                                                 dependencies(dependencies),
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

    esp_err_t ret = esp_event_handler_instance_register_with(
        motor_event_loop,
        MOTOR_EVENTS,
        motor_event_id_t::STATE_TRANSITION_EVENT,
        &state_transition_event_handler,
        this,
        &state_transition_event_handler_instance);

    if (ret != ESP_OK)
    {
        ESP_LOGE(FUNCTION_NAME, "Failed to register event handler: %s", esp_err_to_name(ret));
    }

    ESP_LOGI(FUNCTION_NAME, "Motor ID: %lu -> state_transition_event_handler registered for MOTOR_EVENTS", canId);
    context->transition_ready_state(MotorContext::ReadyState::MOTOR_INITIALIZED);
}

MotorController::~MotorController()
{
    vTaskDelete(task_handle_query_status);
    vTaskDelete(task_handle_send_position);
    esp_event_loop_delete(motor_event_loop);
}

esp_err_t MotorController::start_basic_tasks()
{
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    BaseType_t xRet = xTaskCreatePinnedToCore(
        &MotorController::vTask_query_status,
        "TASK_queryStatus",
        1024 * 4,
        this,
        1,
        &task_handle_query_status,
        tskNO_AFFINITY);

    CHECK_THAT(task_handle_query_status != NULL);
    ESP_LOGI(FUNCTION_NAME, "TASK_queryStatus created");

    return ESP_OK;
}

esp_err_t MotorController::start_timed_tasks()
{
    BaseType_t xReturned;
    ESP_LOGI(FUNCTION_NAME, "Initializing tasks");

    // if (task_handle_query_position == nullptr)
    // {
    //     xReturned = xTaskCreatePinnedToCore(&MotorController::vTask_query_position, "TASK_queryPosition", 1024 * 3, this, 3, &task_handle_query_position, tskNO_AFFINITY);
    //     CHECK_THAT(task_handle_query_position != NULL);
    //     CHECK_THAT(xReturned != NULL);
    //     ESP_LOGI(FUNCTION_NAME, "TASK_queryPosition created");
    // }

    vTaskDelay(2000 / portTICK_PERIOD_MS);
    if (task_handle_send_position == nullptr)
    {
        xReturned = xTaskCreatePinnedToCore(&MotorController::vtask_send_positon, "TASK_SendRandomTargetPositionCommands", 1024 * 3, this, 4, &task_handle_send_position, tskNO_AFFINITY);
        CHECK_THAT(task_handle_send_position != nullptr);
        CHECK_THAT(xReturned == pdPASS);
        ESP_LOGI(FUNCTION_NAME, "TASK_SendRandomTargetPositionCommands created");
    }
    return ESP_OK;
}

void MotorController::stop_timed_tasks()
{
    vTaskDelete(task_handle_query_position);
    vTaskDelete(task_handle_send_position);
}

// void MotorController::post_event(motor_event_id_t event)
// {
//     ESP_ERROR_CHECK(esp_event_post_to(motor_event_loop, MOTOR_EVENTS, MOTOR_TRANSITION_EVENT, (void *)&event, sizeof(event), portMAX_DELAY));
// }

esp_err_t MotorController::query_position()
{
    esp_err_t ret = ESP_FAIL;
    ESP_LOGI(FUNCTION_NAME, "Querying motor position");
    SEND_COMMAND_BY_ID(motor_mutex, command_factory, READ_ENCODER_VALUE_CARRY, context, ret);
    return ret;
}

esp_err_t MotorController::query_status()
{
    esp_err_t ret = ESP_FAIL;
    ESP_LOGI(FUNCTION_NAME, "Querying motor status");
    SEND_COMMAND_BY_ID(motor_mutex, command_factory, QUERY_MOTOR_STATUS, context, ret);
    return ret;
}

esp_err_t MotorController::set_working_current(uint16_t current_ma)
{
    esp_err_t ret = ESP_FAIL;
    auto cmd = command_factory->create_command(SET_WORKING_CURRENT, uint16_t{current_ma});
    SEND_COMMAND_BY_ID_WITH_PAYLOAD(motor_mutex, cmd, context, ret);
    return ret;
}

esp_err_t MotorController::set_target_position()
{
    esp_err_t ret = ESP_FAIL;

    xSemaphoreTake(motor_mutex, portMAX_DELAY);

    // uint32_t rand_pos_value = (esp_random() % 10) * STEPS_PER_REVOLUTION;
    uint32_t rand_pos_value = (esp_random() % 360);
    ESP_LOGI(FUNCTION_NAME, "Random value: %u", static_cast<unsigned int>(rand_pos_value));
    uint32_t position = static_cast<uint32_t>(rand_pos_value);
    ESP_LOGI(FUNCTION_NAME, "Random position: %u", static_cast<unsigned int>(position));
    uint32_t angle_steps = calculate_steps_for_angle(position);
    uint32_t angle_steps_with_ratio = angle_steps * ACTUATOR_GEAR_RATIO;

    ESP_LOGI(FUNCTION_NAME, "Random angle_steps: %u", static_cast<unsigned int>(angle_steps));
    ESP_LOGI(FUNCTION_NAME, "Random angle_steps_with_ratio: %u", static_cast<unsigned int>(angle_steps_with_ratio));

    uint16_t speed = esp_random() % 1600;
    uint8_t acceleration = esp_random() % 255;
    bool absolute = esp_random() % 2;
    absolute = true;
    CommandIds command_id = (absolute) ? RUN_MOTOR_ABSOLUTE_MOTION_BY_AXIS : RUN_MOTOR_RELATIVE_MOTION_BY_AXIS;
    int negative_random = esp_random() % 2;
    if (negative_random && absolute != true)
    {
        position = -position;
    }

    speed = 3000;
    acceleration = 1;
    // position = STEPS_PER_REVOLUTION;

    ESP_LOGI(FUNCTION_NAME, "Sending %s command to move motor at with speed: %d with acceleration of %d to about: %ld°", magic_enum::enum_name(command_id).data(), speed, acceleration, static_cast<uint24_t>(position));
    auto cmd = command_factory->create_command(command_id, speed, acceleration, static_cast<uint24_t>(angle_steps_with_ratio));
    SEND_COMMAND_BY_ID_WITH_PAYLOAD(motor_mutex, cmd, context, ret);

    // auto cmd = command_factory->create_set_target_position_command(absolute);
    // cmd->set_absolute(absolute).set_acceleration(acceleration).set_position(position).set_speed(speed);

    // ret = cmd->execute();
    // if (ret != ESP_OK)
    // {
    //     context->transition_ready_state(MotorContext::ReadyState::MOTOR_ERROR);
    // }

    xSemaphoreGive(motor_mutex);
    return ret;
};
