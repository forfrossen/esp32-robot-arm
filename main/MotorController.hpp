#ifndef MOTORCONTROLLER_H
#define MOTORCONTROLLER_H

#include "CommandMapper.hpp"
#include "Commands/TWAICommandFactory.hpp"
#include "StateMachine.hpp"
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
#include <chrono>
#include <cstdint>
#include <functional>
#include <map>

ESP_EVENT_DECLARE_BASE(MOTOR_CONTROLLER_EVENT);

class MotorController
{
public:
    enum MotorEvent
    {
        MOTOR_EVENT_INIT,
        MOTOR_EVENT_READY,
        MOTOR_EVENT_RUNNING,
        MOTOR_EVENT_ERROR,
        MOTOR_EVENT_RECOVERING
    };

    enum class OperatingState
    {
        UNKNOWN,
        STOPPED,
        ACCELERATING,
        DECELERATING,
        FULL_SPEED,
        HOMING,
        CALIBRATING
    };

    enum MotorState
    {
        MOTOR_INIT,
        MOTOR_READY,
        MOTOR_ERROR,
        MOTOR_RECOVERING
    };

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

    esp_err_t init();
    void handle_response(twai_message_t *msg);
    void handle_received_message(twai_message_t *twai_message_t);
    void handle_query_status_response(twai_message_t *msg);
    void handleQueryMotorPositionResponse(twai_message_t *msg);
    void handleSetPositionResponse(twai_message_t *msg);
    void handeSetHomeResponse(twai_message_t *msg);
    void handleSetWorkModeResponse(twai_message_t *msg);
    void handleSetCurrentResponse(twai_message_t *msg);
    void decodeMessage(twai_message_t *msg);

    esp_err_t set_target_position();
    esp_err_t query_position();
    esp_err_t query_status();

    esp_err_t init_tasks();

    uint32_t get_carry_value() const { return carry_value; }
    uint16_t get_encoder_value() const { return encoder_value; }
    OperatingState get_motor_operating_state() const { return motor_operating_state; }
    std::chrono::system_clock::time_point get_last_seen() const { return last_seen; }

    MotorControllerFSM::State get_state() const { return fsm_moving.get_state(); }

    void set_state(MotorControllerFSM::State new_state);

private:
    std::chrono::system_clock::time_point last_seen;
    int error_counter = 0;
    OperatingState motor_operating_state = OperatingState::UNKNOWN;
    SemaphoreHandle_t motor_mutex = xSemaphoreCreateMutex();

    uint32_t canId;
    std::shared_ptr<TWAICommandFactory> command_factory;
    QueueHandle_t inQ;
    QueueHandle_t outQ;
    std::shared_ptr<CommandMapper> command_mapper;
    std::shared_ptr<CommandLifecycleRegistry> command_lifecycle_registry;

    MotorControllerFSM fsm_moving;

    MotorState fsm_motor_state = MotorState::MOTOR_INIT;
    MotorState get_motor_state();
    void transition_motor_state(MotorState new_state);

    uint32_t carry_value;
    uint16_t encoder_value;
    std::string F5Status;

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

    EventGroupHandle_t motor_event_group;
    EventGroupHandle_t system_event_group;
};

#endif // CANSERVO_H