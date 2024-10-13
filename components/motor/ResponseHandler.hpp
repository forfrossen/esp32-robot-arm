#ifndef MOTORRESPONSEHANDLER_H
#define MOTORRESPONSEHANDLER_H

#include "Context.hpp"
#include "Controller.hpp"
#include "MksEnums.hpp"
#include "TypeDefs.hpp"
#include "esp_log.h"
#include "utils.hpp"
#include <driver/twai.h>

class ResponseHandler
{
public:
    ResponseHandler(uint32_t canId, std::shared_ptr<MotorContext> context, std::shared_ptr<EventLoops> event_loops);
    ~ResponseHandler();

    void process_message(twai_message_t *msg);

    void handle_generic_response(twai_message_t *msg);
    void handle_read_uint16_response(twai_message_t *msg);
    void handle_set_command_response(twai_message_t *msg);
    void handle_query_status_response(twai_message_t *msg);
    void handle_query_motor_position_response(twai_message_t *msg);
    void handle_query_motor_speed_response(twai_message_t *msg);
    void handle_set_position_response(twai_message_t *msg);
    void handle_set_home_response(twai_message_t *msg);
    void handle_set_work_mode_response(twai_message_t *msg);
    void handle_set_current_response(twai_message_t *msg);

private:
    uint32_t canId;
    std::shared_ptr<MotorContext> context;
    esp_event_loop_handle_t system_event_loop;

    bool
    is_response_error(twai_message_t *msg);

    void handle_message_error();
    void handle_received_no_error();
    void check_for_error_and_do_transition(twai_message_t *msg);
    void print_unknown_response_code(twai_message_t *msg);

    void log_twai_message(twai_message_t *msg);
    esp_event_handler_instance_t incoming_message_event_handler_instance;

    static void incoming_message_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data);
};

#endif // MOTORRESPONSEHANDLER_H