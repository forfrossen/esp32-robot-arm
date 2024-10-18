#ifndef MOTORRESPONSEHANDLER_H
#define MOTORRESPONSEHANDLER_H

#include "Context.hpp"
#include "Controller.hpp"
#include "MksEnums.hpp"
#include "TypeDefs.hpp"
#include "esp_log.h"
#include "utils.hpp"
#include <driver/twai.h>

#include "../response_handler/ResponseHandlerBase.hpp"

#include "../response_handler/CommandLifecycleHandler.hpp"
#include "../response_handler/ErrorCheckHandler.hpp"
#include "../response_handler/LogMessageHandler.hpp"
#include "../response_handler/ReadyStateTransitionHandler.hpp"
#include "../response_handler/ResponseDataHandler.hpp"
#include "../response_handler/ResponseHandlerEntry.hpp"

class ResponseHandler
{
public:
    ResponseHandler(uint32_t canId, std::shared_ptr<MotorContext> context, std::shared_ptr<EventLoops> event_loops);
    ~ResponseHandler();

private:
    uint32_t canId;
    std::shared_ptr<MotorContext> context;
    esp_event_loop_handle_t system_event_loop;
    esp_event_loop_handle_t motor_event_loop;
    esp_event_handler_instance_t incoming_message_event_handler_instance;

    std::shared_ptr<ResponseHandlerEntry> entry_point;
    std::shared_ptr<LogMessageHandler> log_handler;
    std::shared_ptr<ReadyStateTransitionHandler> state_handler;
    std::shared_ptr<ErrorCheckHandler> error_handler;
    std::shared_ptr<CommandLifecycleHandler> lifecycle_handler;
    std::shared_ptr<ResponseDataHandler> data_handler;

    bool is_response_error(twai_message_t *msg);

    esp_err_t process_message(twai_message_t *msg);
    esp_err_t handle_message_error();
    esp_err_t handle_received_no_error();
    esp_err_t check_for_error_and_do_transition(twai_message_t *msg);

    esp_err_t handle_generic_response(twai_message_t *msg);
    esp_err_t handle_read_uint16_response(twai_message_t *msg);
    esp_err_t handle_set_command_response(twai_message_t *msg);
    esp_err_t handle_query_status_response(twai_message_t *msg);
    esp_err_t handle_query_motor_position_response(twai_message_t *msg);
    esp_err_t handle_query_motor_speed_response(twai_message_t *msg);
    esp_err_t handle_set_position_response(twai_message_t *msg);
    esp_err_t handle_set_home_response(twai_message_t *msg);
    esp_err_t handle_set_work_mode_response(twai_message_t *msg);
    esp_err_t handle_set_current_response(twai_message_t *msg);

    esp_err_t print_unknown_response_code(twai_message_t *msg);

    static void incoming_message_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data);
};

#endif // MOTORRESPONSEHANDLER_H