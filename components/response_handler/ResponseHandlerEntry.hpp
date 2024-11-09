#ifndef RESPONSE_HANDLER_ENTRY_HPP
#define RESPONSE_HANDLER_ENTRY_HPP

#include "hal/twai_types.h"
#include <esp_log.h>

#include "ResponseHandlerBase.hpp"
#include "utils.hpp"

class ResponseHandlerEntry : public ResponseHandlerBase
{
public:
    bool handle_response(const twai_message_t &msg) override
    {
        ESP_LOGD(FUNCTION_NAME, "Entry Point Reached");
        // Pass to the next handler
        return ResponseHandlerBase::handle_response(msg);
    }
};

#endif // RESPONSE_HANDLER_ENTRY_HPP