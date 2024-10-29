#ifndef ERROR_CHECK_HANDLER_HPP
#define ERROR_CHECK_HANDLER_HPP

#include "ResponseHandlerBase.hpp"
#include "esp_log.h"
#include "utils.hpp"

class ErrorCheckHandler : public ResponseHandlerBase
{
public:
    ErrorCheckHandler(std::shared_ptr<MotorContext> context, std::shared_ptr<CommandLifecycleRegistry> registry) : ResponseHandlerBase()
    {
        this->context = context;
        this->registry = registry;
    }

    bool handle_response(const twai_message_t &msg) override
    {
        ESP_LOGD(FUNCTION_NAME, "called");
        bool status_in_data1 = IS_STATUS_IN_DATA1(static_cast<CommandIds>(msg.data[0]));
        if (status_in_data1)
        {
            ESP_LOGD(FUNCTION_NAME, "Command follows status in data1 rule.");
            if (is_response_error(msg))
            {
                ESP_LOGW(FUNCTION_NAME, "Error response received.");
                esp_err_t ret = on_error(msg);
                assert(ret == ESP_OK);
            }
            else
            {
                ESP_LOGD(FUNCTION_NAME, "Response does not contain an error.");
            }
        }
        return ResponseHandlerBase::handle_response(msg);
    }

private:
    std::shared_ptr<MotorContext> context;
    std::shared_ptr<CommandLifecycleRegistry> registry;

    bool is_response_error(const twai_message_t &msg)
    {
        return (msg.data[1] == 0);
    }

    esp_err_t on_error(const twai_message_t &msg)
    {
        ESP_LOGD(FUNCTION_NAME, "Transitioning ReadyState to MOTOR_ERROR");
        registry->update_command_state(msg.identifier, msg.data[0]);

        CHECK_THAT(context->transition_ready_state(MotorContext::ReadyState::MOTOR_ERROR) == ESP_OK);

        return ESP_OK;
    }

    esp_err_t on_no_error(const twai_message_t &msg)
    {
        ESP_LOGD(FUNCTION_NAME, "Transitioning ReadyState to MOTOR_READY");

        CHECK_THAT(context->transition_ready_state(MotorContext::ReadyState::MOTOR_READY) == ESP_OK);

        return ESP_OK;
    }
};

#endif // ERROR_CHECK_HANDLER_HPP