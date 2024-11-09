#ifndef MOTOR_DIRECTOR_HPP
#define MOTOR_DIRECTOR_HPP

#include "MksEnums.hpp"
#include "TypeDefs.hpp"

#include "CommandFactory.hpp"
#include "CommandLifecycleRegistry.hpp"
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

#include <cstdint>
#include <functional>
#include <map>
#include <memory>

class MotorBuilder
{
public:
    MotorBuilder(
        uint32_t id,
        std::shared_ptr<TWAIController> twai_controller,
        std::shared_ptr<CommandLifecycleRegistry> command_lifecyle_registry,
        esp_event_loop_handle_t system_event_loop,
        EventGroupHandle_t &system_event_group);
    ~MotorBuilder();

    esp_err_t build_dependencies();

    std::unique_ptr<MotorController> get_result();

private:
    uint32_t canId;

    std::shared_ptr<TWAIController> twai_controller;
    std::shared_ptr<CommandLifecycleRegistry> command_lifecyle_registry;

    esp_event_loop_handle_t system_event_loop;
    EventGroupHandle_t system_event_group;

    EventGroupHandle_t motor_event_group;
    esp_event_loop_args_t motor_loop_args;
    esp_event_loop_handle_t motor_event_loop;

    esp_event_handler_instance_t state_transition_event_handler_instance;
    SemaphoreHandle_t motor_mutex;

    std::shared_ptr<MotorContext> context;
    std::shared_ptr<ResponseHandler> response_handler;
    std::shared_ptr<CommandFactorySettings> settings;
    std::shared_ptr<CommandFactory> command_factory;

    std::shared_ptr<MotorControllerDependencies> dependencies;
    std::shared_ptr<event_loops_t> event_loops;
    std::shared_ptr<event_groups_t> event_groups;
    std::shared_ptr<MotorContext> motor_context;
    std::shared_ptr<ResponseHandler> motor_response_handler;

    esp_err_t build_context();
    esp_err_t build_response_handler();
    esp_err_t build_command_factory_settings();
    esp_err_t build_command_factory();
    esp_err_t build_motor_loop_args();
    esp_err_t build_motor_loop();
    esp_err_t build_motor_mutex();
    esp_err_t build_event_loops_container();
    esp_err_t build_event_groups_container();
    esp_err_t build_dependencies_container();

    void post_event(motor_event_id_t event);
};

#endif // MOTOR_DIRECTOR_HPP