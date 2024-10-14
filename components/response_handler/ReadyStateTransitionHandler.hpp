
#include "ResponseHandlerBase.hpp"
#include "esp_log.h"
#include "utils.hpp"

#ifndef READY_STATE_TRANSITION_HANDLER_HPP
#define READY_STATE_TRANSITION_HANDLER_HPP

class ReadyStateTransitionHandler : public ResponseHandlerBase
{
public:
    ReadyStateTransitionHandler(std::shared_ptr<MotorContext> context) : ResponseHandlerBase()
    {
        this->context = context;
    }

    bool handle_response(const twai_message_t &msg) override
    {
        ESP_LOGI(FUNCTION_NAME, "called");
        esp_err_t ret = do_transition(msg);

        return ResponseHandlerBase::handle_response(msg);
    }

private:
    std::shared_ptr<MotorContext> context;
    esp_err_t do_transition(const twai_message_t &msg)
    {
        ESP_LOGI(FUNCTION_NAME, "called");
        if (context->is_recovering() || context->is_init())
        {
            CHECK_THAT(context->transition_ready_state(MotorContext::ReadyState::MOTOR_READY) == ESP_OK);
            return ESP_OK;
        }
        return ESP_OK;
    }
};

#endif // READY_STATE_TRANSITION_HANDLER_HPP