
#include "Builder.hpp"

static const char *TAG = "MotorBuilder";

MotorBuilder::MotorBuilder(
    uint32_t id,
    std::shared_ptr<TWAIController> twai_controller,
    std::shared_ptr<CommandLifecycleRegistry> command_lifecyle_registry,
    esp_event_loop_handle_t system_event_loop,
    EventGroupHandle_t &system_event_group)
    : canId(id),
      twai_controller(twai_controller),
      command_lifecyle_registry(command_lifecyle_registry),
      system_event_loop(system_event_loop),
      system_event_group(system_event_group)
{
    ESP_LOGD(TAG, "New MotorBuilder-Object created with CAN ID: %lu", canId);
}

MotorBuilder::~MotorBuilder()
{
    ESP_LOGD(TAG, "Destructor called for MotorBuilder with CAN ID: %lu", canId);
    if (dependencies != nullptr)
    {
        ESP_LOGD(TAG, "Deleting dependencies for MotorBuilder with CAN ID: %lu", canId);
        dependencies.reset();
    }
    ESP_LOGD(TAG, "MotorBuilder with CAN ID: %lu deleted", canId);
}

esp_err_t MotorBuilder::build_dependencies()
{
    esp_err_t ret = ESP_OK;

    ret = build_motor_loop_args();
    ESP_RETURN_ON_ERROR(ret, TAG, "Failed to build motor loop args for motor %lu", canId);

    ret = build_motor_loop();
    ESP_RETURN_ON_ERROR(ret, TAG, "Failed to build motor loop for motor %lu", canId);

    ret = build_event_loops_container();
    ESP_RETURN_ON_ERROR(ret, TAG, "Failed to build event loops container for motor %lu", canId);

    ret = build_event_groups_container();
    ESP_RETURN_ON_ERROR(ret, TAG, "Failed to build event groups container for motor %lu", canId);

    ret = build_motor_mutex();
    ESP_RETURN_ON_ERROR(ret, TAG, "Failed to build motor mutex for motor %lu", canId);

    ret = twai_controller->register_motor_id(canId, motor_event_loop);
    ESP_RETURN_ON_ERROR(ret, TAG, "Failed to register motorId %lu at twai_controller", canId);

    ret = command_lifecyle_registry->register_new_motor(canId);
    ESP_RETURN_ON_ERROR(ret, TAG, "Failed register motorId %lu at command_lifecyle_registry", canId);

    ret = build_command_factory_settings();
    ESP_RETURN_ON_ERROR(ret, TAG, "Failed to build command factory settings for motor %lu", canId);

    ret = build_command_factory();
    ESP_RETURN_ON_ERROR(ret, TAG, "Failed to build command factory for motor %lu", canId);

    ret = build_context();
    ESP_RETURN_ON_ERROR(ret, TAG, "Failed to build context for motor %lu", canId);

    ret = build_response_handler();
    ESP_RETURN_ON_ERROR(ret, TAG, "Failed to build response handler for motor %lu", canId);

    ret = build_dependencies_container();
    ESP_RETURN_ON_ERROR(ret, TAG, "Failed to build dependencies container for motor %lu", canId);

    return ESP_OK;
}

std::unique_ptr<MotorController> MotorBuilder::get_result()
{
    ESP_LOGD(TAG, "Building MotorController with CAN ID: %lu", canId);
    assert(dependencies != nullptr);
    return std::make_unique<MotorController>(dependencies);
}

esp_err_t MotorBuilder::build_context()
{
    ESP_LOGD(TAG, "Building MotorContext for MotorController with CAN ID: %lu", canId);
    CHECK_THAT(event_loops != nullptr);
    motor_context = std::make_shared<MotorContext>(canId, event_loops);
    CHECK_THAT(motor_context != nullptr);
    return ESP_OK;
}

esp_err_t MotorBuilder::build_response_handler()
{
    ESP_LOGD(TAG, "Building ResponseHandler for MotorController with CAN ID: %lu", canId);
    CHECK_THAT(command_lifecyle_registry != nullptr);
    CHECK_THAT(motor_context != nullptr);
    CHECK_THAT(event_loops != nullptr);
    response_handler = std::make_shared<ResponseHandler>(canId, motor_context, event_loops, command_lifecyle_registry);
    CHECK_THAT(response_handler != nullptr);
    return ESP_OK;
}

esp_err_t MotorBuilder::build_command_factory_settings()
{
    ESP_LOGD(TAG, "Building CommandFactorySettings for MotorController with CAN ID: %lu", canId);
    CHECK_THAT(command_lifecyle_registry != nullptr);
    CHECK_THAT(system_event_loop != nullptr);
    settings = std::make_shared<CommandFactorySettings>(canId, system_event_loop, command_lifecyle_registry);
    CHECK_THAT(settings != nullptr);
    return ESP_OK;
}

esp_err_t MotorBuilder::build_command_factory()
{
    ESP_LOGD(TAG, "Building CommandFactory for MotorController with CAN ID: %lu", canId);
    CHECK_THAT(settings != nullptr);
    command_factory = std::make_shared<CommandFactory>(settings);
    return ESP_OK;
}

esp_err_t MotorBuilder::build_motor_loop_args()
{
    std::string task_name_str = "motor_event_task_" + std::to_string(canId);
    motor_loop_args = {
        .queue_size = 5,
        .task_name = task_name_str.c_str(),
        .task_priority = 5,
        .task_stack_size = 1024 * 3,
        .task_core_id = tskNO_AFFINITY};
    return ESP_OK;
}

esp_err_t MotorBuilder::build_motor_loop()
{
    ESP_LOGD(TAG, "Building MotorEventLoop for MotorController with CAN ID: %lu", canId);
    esp_err_t ret = ESP_OK;
    esp_event_loop_handle_t tmp_handle;
    ret = esp_event_loop_create(&motor_loop_args, &motor_event_loop);
    CHECK_THAT(ret == ESP_OK);
    CHECK_THAT(motor_event_loop != nullptr);
    return ESP_OK;
}

esp_err_t MotorBuilder::build_motor_mutex()
{
    ESP_LOGD(TAG, "Building MotorMutex for MotorController with CAN ID: %lu", canId);
    motor_mutex = xSemaphoreCreateMutex();
    CHECK_THAT(motor_mutex != NULL);
    return ESP_OK;
}

esp_err_t MotorBuilder::build_event_loops_container()
{
    ESP_LOGD(TAG, "Building EventLoops container for MotorController with CAN ID: %lu", canId);
    esp_err_t ret = ESP_OK;
    CHECK_THAT(motor_event_loop != NULL);
    CHECK_THAT(system_event_loop != NULL);
    event_loops = std::make_shared<EventLoops>(system_event_loop, motor_event_loop);
    CHECK_THAT(event_loops != nullptr);
    return ESP_OK;
}

esp_err_t MotorBuilder::build_event_groups_container()
{
    ESP_LOGD(TAG, "Building EventGroups container for MotorController with CAN ID: %lu", canId);
    motor_event_group = xEventGroupCreate();
    CHECK_THAT(motor_event_group != NULL);
    CHECK_THAT(system_event_group != NULL);
    event_groups = std::make_shared<EventGroups>(system_event_group, motor_event_group);
    CHECK_THAT(event_groups != nullptr);
    return ESP_OK;
}

esp_err_t MotorBuilder::build_dependencies_container()
{
    ESP_LOGD(TAG, "Building MotorControllerDependencies for MotorController with CAN ID: %lu", canId);
    dependencies = std::make_shared<MotorControllerDependencies>(
        canId,
        motor_mutex,
        event_groups,
        event_loops,
        command_lifecyle_registry,
        command_factory,
        motor_context,
        response_handler);
    CHECK_THAT(dependencies != nullptr);
    return ESP_OK;
}