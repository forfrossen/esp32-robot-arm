#ifndef MOTORRESPONSEHANDLER_H
#define MOTORRESPONSEHANDLER_H

#include "MksEnums.hpp"
#include "MotorContext.hpp"
#include "TypeDefs.hpp"
#include "esp_log.h"
#include "utils.hpp"
#include <driver/twai.h>

class MotorResponseHandler
{
public:
    MotorResponseHandler(uint32_t canId, std::shared_ptr<MotorContext> context) : canId(canId), context(context) {};
    ~MotorResponseHandler();

    void process_message(twai_message_t *msg);
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

    bool is_response_error(twai_message_t *msg);

    void handle_message_error();
    void handle_received_no_error();
    void check_for_error_and_do_transition(twai_message_t *msg);
    void print_unknown_response_code(twai_message_t *msg);
    void log_twai_message(twai_message_t *msg);
};

#endif // MOTORRESPONSEHANDLER_H