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
    ESP_LOGD(TAG, "New MotorController-Object created with CAN ID: %lu", canId);

    esp_err_t ret = esp_event_handler_instance_register_with(
        motor_event_loop,
        MOTOR_EVENTS,
        motor_event_id_t::STATE_TRANSITION_EVENT,
        &on_state_transition,
        this,
        &state_transition_event_handler_instance);

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to register event handler: %s", esp_err_to_name(ret));
    }

    ESP_LOGD(TAG, "Motor ID: %lu -> on_state_transition registered for MOTOR_EVENTS", canId);
    context->transition_ready_state(MotorContext::ReadyState::MOTOR_INITIALIZED);
}

MotorController::~MotorController()
{
    vTaskDelete(th_query_status);
    vTaskDelete(th_send_position);
    esp_event_loop_delete(motor_event_loop);
}

esp_err_t MotorController::handle_initialize()
{
    ESP_LOGD(TAG, "Handle initialize transition event for motor controller with Id: %lu", canId);
    esp_err_t ret = ESP_OK;
    ret = start_basic_tasks();
    ret == ESP_OK ? context->transition_ready_state(MotorContext::ReadyState::MOTOR_READY)
                  : context->transition_ready_state(MotorContext::ReadyState::MOTOR_ERROR);
    ESP_RETURN_ON_ERROR(ret, TAG, "Error starting basic tasks. Error: %s", esp_err_to_name(ret));
    return ret;
}

esp_err_t MotorController::handle_recover()
{
    ESP_LOGD(TAG, "Handle recovery event for motor controller with Id: %lu", canId);
    esp_err_t ret = ESP_OK;
    ret = start_basic_tasks();
    ret == ESP_OK ? context->transition_ready_state(MotorContext::ReadyState::MOTOR_READY)
                  : context->transition_ready_state(MotorContext::ReadyState::MOTOR_ERROR);
    ESP_RETURN_ON_ERROR(ret, TAG, "Error starting basic tasks. Error: %s", esp_err_to_name(ret));
    return ret;
}

esp_err_t MotorController::handle_error()
{
    ESP_LOGD(TAG, "Handle error event transition for motor with Id: %lu", canId);
    xEventGroupClearBits(motor_event_group, MOTOR_READY_BIT);
    xEventGroupSetBits(motor_event_group, MOTOR_ERROR_BIT);
    context->transition_ready_state(MotorContext::ReadyState::MOTOR_RECOVERING);
    stop_timed_tasks();
    ESP_LOGD(TAG, "Error state for motor %lu", canId);
    return ESP_OK;
}

esp_err_t MotorController::handle_ready()
{
    ESP_LOGD(TAG, "Handle ready event transition for motor with Id:  %lu", canId);
    esp_err_t ret = ESP_OK;
    xEventGroupClearBits(motor_event_group, MOTOR_ERROR_BIT);
    xEventGroupSetBits(motor_event_group, MOTOR_READY_BIT);
    ret = start_timed_tasks();
    if (ret != ESP_OK)
    {
        context->transition_ready_state(MotorContext::ReadyState::MOTOR_ERROR);
    }

    ESP_LOGD(TAG, "Ready state for motor %lu", canId);
    return ESP_OK;
}

esp_err_t MotorController::start_basic_tasks()
{
    eTaskState task_state;
    esp_err_t ret;
    vTaskDelay(3000 / portTICK_PERIOD_MS);

    /**
     * Task: Query status
     */
    ret = get_task_state_without_panic(th_query_status, task_state);
    BaseType_t xReturned = xTaskCreatePinnedToCore(
        &MotorController::vTask_query_status,
        "TASK_queryStatus",
        1024 * 4,
        this,
        1,
        &th_query_status,
        tskNO_AFFINITY);
    CHECK_THAT(xReturned == pdPASS);
    CHECK_THAT(th_query_status != NULL);
    ret = get_task_state_without_panic(th_query_status, task_state);
    CHECK_THAT(ret == ESP_OK);

    return ESP_OK;
}

esp_err_t MotorController::start_timed_tasks()
{
    eTaskState task_state;
    ESP_LOGD(TAG, "Initializing tasks");

    vTaskDelay(2000 / portTICK_PERIOD_MS);

    /**
     * Task: Query position
     */
    get_task_state_without_panic(th_query_position, task_state);
    if (task_state == eDeleted || task_state == eInvalid)
    {
        BaseType_t xReturned = xTaskCreatePinnedToCore(
            &MotorController::vTask_query_position,
            "TASK_queryPosition",
            1024 * 3,
            this,
            3,
            &th_query_position,
            tskNO_AFFINITY);
        CHECK_THAT(xReturned == pdPASS);
        get_task_state_without_panic(th_query_position, task_state);
    }

    /**
     * Task: Send position
     */
    ESP_LOGD(TAG, "Task handle query position is in state: %d", task_state);
    get_task_state_without_panic(th_send_position, task_state);
    if (task_state == eDeleted || task_state == eInvalid)
    {
        BaseType_t xReturned = xTaskCreatePinnedToCore(
            &MotorController::vtask_send_positon,
            "TASK_queryPosition",
            1024 * 3,
            this,
            3,
            &th_send_position,
            tskNO_AFFINITY);
        CHECK_THAT(th_send_position != nullptr);
        CHECK_THAT(xReturned == pdPASS);
        get_task_state_without_panic(th_send_position, task_state);
    }

    ESP_LOGD(TAG, "Task handle query position is in state: %d", task_state);

    return ESP_OK;
}

void MotorController::stop_timed_tasks()
{
    vTaskDelete(th_query_position);
    vTaskDelete(th_send_position);
}

// void MotorController::post_event(motor_event_id_t event)
// {
//     ESP_ERROR_CHECK(esp_event_post_to(motor_event_loop, MOTOR_EVENTS, MOTOR_TRANSITION_EVENT, (void *)&event, sizeof(event), portMAX_DELAY));
// }

esp_err_t MotorController::query_status()
{
    esp_err_t ret = ESP_FAIL;
    ESP_LOGD(TAG, "Querying motor status");
    SEND_COMMAND_BY_ID(command_factory, QUERY_MOTOR_STATUS, context, ret);
    return ret;
}

esp_err_t MotorController::query_position()
{
    esp_err_t ret = ESP_FAIL;
    ESP_LOGD(TAG, "Querying motor position");
    SEND_COMMAND_BY_ID(command_factory, READ_ENCODER_VALUE_CARRY, context, ret);
    return ret;
}

esp_err_t MotorController::set_working_current(uint16_t current_ma)
{
    esp_err_t ret = ESP_FAIL;
    auto cmd = command_factory->create_command(SET_WORKING_CURRENT, uint16_t{current_ma});
    SEND_COMMAND_BY_ID_WITH_PAYLOAD(cmd, context, ret);
    return ret;
}

esp_err_t MotorController::set_target_position()
{
    esp_err_t ret = ESP_FAIL;

    xSemaphoreTake(motor_mutex, portMAX_DELAY);

    // uint32_t rand_pos_value = (esp_random() % 10) * STEPS_PER_REVOLUTION;
    uint32_t rand_pos_value = (esp_random() % 360);
    ESP_LOGD(TAG, "Random value: %u", static_cast<unsigned int>(rand_pos_value));
    uint32_t position = static_cast<uint32_t>(rand_pos_value);
    ESP_LOGD(TAG, "Random position: %u", static_cast<unsigned int>(position));
    uint32_t angle_steps = calculate_steps_for_angle(position);
    uint32_t angle_steps_with_ratio = angle_steps * ACTUATOR_GEAR_RATIO;

    ESP_LOGD(TAG, "Random angle_steps: %u", static_cast<unsigned int>(angle_steps));
    ESP_LOGD(TAG, "Random angle_steps_with_ratio: %u", static_cast<unsigned int>(angle_steps_with_ratio));

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

    ESP_LOGD(TAG, "Sending %s command to move motor at with speed: %d with acceleration of %d to about: %ld°", magic_enum::enum_name(command_id).data(), speed, acceleration, static_cast<uint24_t>(position));
    auto cmd = command_factory->create_command(command_id, speed, acceleration, static_cast<uint24_t>(angle_steps_with_ratio));
    SEND_COMMAND_BY_ID_WITH_PAYLOAD(cmd, context, ret);

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
