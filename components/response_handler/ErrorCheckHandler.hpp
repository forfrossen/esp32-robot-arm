#include "ResponseHandlerBase.hpp"
#include "esp_log.h"
#include "utils.hpp"

#ifndef ERROR_CHECK_HANDLER_HPP
#define ERROR_CHECK_HANDLER_HPP

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
        if (is_response_error(msg))
        {
            ESP_LOGW(FUNCTION_NAME, "Error response received.");
            esp_err_t ret = on_error();
            assert(ret == ESP_OK);
        }

        // No error, pass to the next handler
        return ResponseHandlerBase::handle_response(msg);
    }

private:
    std::shared_ptr<MotorContext> context;
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