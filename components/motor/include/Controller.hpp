#ifndef MOTORCONTROLLER_H
#define MOTORCONTROLLER_H

#include "MksEnums.hpp"
#include "TypeDefs.hpp"

#include "CommandFactory.hpp"
#include "CommandPayloadTypeDefs.hpp"
#include "CommandStateMachine.hpp"
#include "Context.hpp"
#include "ResponseHandler.hpp"
#include "TWAIController.hpp"
#include "utils.hpp"
#include <esp_check.h>
#include <esp_err.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_random.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <cstdint>
#include <functional>
#include <map>

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

private:
    uint32_t canId;
    std::shared_ptr<MotorControllerDependencies> dependencies;
    std::shared_ptr<CommandFactory> command_factory;
    QueueHandle_t inQ;
    QueueHandle_t outQ;
    EventGroupHandle_t system_event_group;
    EventGroupHandle_t motor_event_group;
    esp_event_loop_handle_t system_event_loop;
    esp_event_loop_handle_t motor_event_loop;
    esp_event_handler_instance_t state_transition_event_handler_instance;
    SemaphoreHandle_t motor_mutex;
    std::shared_ptr<CommandLifecycleRegistry> command_lifecycle_registry;
    std::shared_ptr<MotorContext> context;
    std::shared_ptr<ResponseHandler> response_handler;

    int error_counter = 0;

    void post_event(motor_event_id_t event);

    esp_err_t start_basic_tasks();
    esp_err_t start_timed_tasks();
    void stop_timed_tasks();
    void stop_basic_tasks();

    static void on_state_transition(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
    esp_err_t handle_initialize();
    esp_err_t handle_recover();
    esp_err_t handle_error();
    esp_err_t handle_ready();

    esp_err_t set_target_position();
    esp_err_t query_position();
    esp_err_t query_status();
    esp_err_t set_working_current(uint16_t currentMa);

    TaskHandle_t th_handle_inQ = nullptr;
    static void vTask_handleInQ(void *pvParameters);

    TaskHandle_t th_query_position = nullptr;
    static void vTask_query_position(void *pvParameters);

    TaskHandle_t th_send_position = nullptr;
    static void vtask_send_positon(void *pvParameters);

    TaskHandle_t th_query_status = nullptr;
    static void vTask_query_status(void *pvParameters);
};

#endif // CANSERVO_H