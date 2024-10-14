
#include "ResponseHandlerBase.hpp"
#include "esp_log.h"
#include "utils.hpp"

#ifndef RESPONSE_HANDLER_ENTRY_HPP

class ResponseHandlerEntry : public ResponseHandlerBase
{
public:
    bool handle_response(const twai_message_t &msg) override
    {
        ESP_LOGI(FUNCTION_NAME, "Entry Point Reached");
        // Pass to the next handler
        return ResponseHandlerBase::handle_response(msg);
    }
};

#endif // RESPONSE_HANDLER_ENTRY_HPP