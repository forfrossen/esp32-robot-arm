#ifndef ERROR_CHECK_HANDLER_HPP
#define ERROR_CHECK_HANDLER_HPP

#include "ResponseHandlerBase.hpp"
#include "esp_log.h"
#include "utils.hpp"


class ErrorCheckHandler : public ResponseHandlerBase
{
public:
    ErrorCheckHandler(std::shared_ptr<MotorContext> context) : ResponseHandlerBase()
    {
        this->context = context;
    }

    bool handle_response(const twai_message_t &msg) override
    {
        ESP_LOGI(FUNCTION_NAME, "called");
        if (follows_status_in_data1_rule(static_cast<CommandIds>(msg.data[0])))
        {
            ESP_LOGI(FUNCTION_NAME, "Command follows status in data1 rule.");
            if (is_response_error(msg))
            {
                ESP_LOGW(FUNCTION_NAME, "Error response received.");
                esp_err_t ret = on_error();
                assert(ret == ESP_OK);
            }
        }
        return ResponseHandlerBase::handle_response(msg);
    }

private:
    std::shared_ptr<MotorContext> context;

    bool follows_status_in_data1_rule(CommandIds command_id)
    {
        switch (command_id)
        {
        // These commands don't follow the rule, so return false
        case CommandIds::READ_ENCODER_VALUE_CARRY:
        case CommandIds::READ_ENCODED_VALUE_ADDITION:
        case CommandIds::READ_MOTOR_SPEED:
        case CommandIds::READ_NUM_PULSES_RECEIVED:
        case CommandIds::READ_MOTOR_SHAFT_ANGLE_ERROR:
            return false;

        // All other commands follow the rule
        default:
            return true;
        }
    }

    bool is_response_error(const twai_message_t &msg)
    {
        return (msg.data[1] == 0);
    }

    esp_err_t on_error()
    {
        ESP_LOGI(FUNCTION_NAME, "Transitioning ReadyState to MOTOR_ERROR");
        CHECK_THAT(context->transition_ready_state(MotorContext::ReadyState::MOTOR_ERROR) == ESP_OK);
        return ESP_OK;
    }
};

#endif // ERROR_CHECK_HANDLER_HPP