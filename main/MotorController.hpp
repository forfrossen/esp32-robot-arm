#ifndef MOTORCONTROLLER_H
#define MOTORCONTROLLER_H

#include "CommandMapper.hpp"
#include "CommandStateMachine.hpp"
#include "Commands/TWAICommandFactory.hpp"
#include "MotorContext.hpp"
#include "MotorResponseHandler.hpp"
#include "TWAIController.hpp"
#include "TypeDefs.hpp"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "utils.hpp"

#include <cstdint>
#include <functional>
#include <map>

ESP_EVENT_DECLARE_BASE(MOTOR_CONTROLLER_EVENT);

class MotorController
{
public:
    enum MotorError
    {
        NO_ERROR,
        CONNECTION_LOST,
        MOTOR_HIT_LIMIT,
        OVERCURRENT_DETECTED,
        UNKNOWN_ERROR
    };

    MotorController(std::shared_ptr<MotorControllerDependencies> dependencies);
    ~MotorController();

    esp_err_t init_tasks();

    esp_err_t set_target_position();
    esp_err_t query_position();
    esp_err_t query_status();

    CommandStateMachine::CommandState get_command_state() const;
    void set_command_state(CommandStateMachine::CommandState new_state);

private:
    uint32_t canId;
    std::shared_ptr<TWAICommandFactory> command_factory;
    QueueHandle_t inQ;
    QueueHandle_t outQ;
    EventGroupHandle_t motor_event_group;
    EventGroupHandle_t system_event_group;
    SemaphoreHandle_t motor_mutex;
    std::shared_ptr<CommandMapper> command_mapper;
    std::shared_ptr<CommandLifecycleRegistry> command_lifecycle_registry;
    std::shared_ptr<MotorContext> context;
    std::shared_ptr<MotorResponseHandler> response_handler;

    int error_counter = 0;

        esp_err_t execute_query_command(std::function<TWAICommandBuilderBase<GenericCommandBuilder> *()> command_factory_method);

    esp_event_loop_handle_t motor_controller_event_loop_handle;
    static void motor_controller_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data);
    void post_event(MotorEvent event);
    void init_event_loop();

    TaskHandle_t task_handle_handle_inQ;
    static void vTask_handleInQ(void *pvParameters);

    TaskHandle_t task_handle_query_position;
    static void vTask_query_position(void *pvParameters);

    TaskHandle_t task_handle_send_position;
    static void vtask_send_positon(void *pvParameters);

    TaskHandle_t task_handle_query_status;
    static void vTask_query_status(void *pvParameters);

    TaskHandle_t task_handle_handle_unhealthy;
    static void vTask_handle_unhealthy(void *pvParameters);
};

#endif // CANSERVO_H