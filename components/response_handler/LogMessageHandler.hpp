#ifndef LOG_MESSAGE_HANDLER_HPP
#define LOG_MESSAGE_HANDLER_HPP

#include "ResponseHandlerBase.hpp"
#include "esp_log.h"
#include "utils.hpp"

class LogMessageHandler : public ResponseHandlerBase
{
public:
    bool handle_response(const twai_message_t &msg) override
    {
        ESP_LOGD(FUNCTION_NAME, "called");
        log_twai_message(&msg, true);
        // Pass to the next handler
        return ResponseHandlerBase::handle_response(msg);
    }
};

#endif // LOG_MESSAGE_HANDLER_HPP