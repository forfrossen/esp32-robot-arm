#ifndef COMMAND_LIFECYCLE_HANDLER_HPP
#define COMMAND_LIFECYCLE_HANDLER_HPP

#include "ResponseHandlerBase.hpp"
#include "utils.hpp"
#include <esp_log.h>

class CommandLifecycleHandler : public ResponseHandlerBase
{
public:
    bool handle_response(const twai_message_t &msg) override
    {
        // Update the lifecycle of the command (e.g., mark it as completed)
        ESP_LOGD("CommandLifecycleHandler", "Updating lifecycle of the sent command.");
        // Pass to the next handler
        return ResponseHandlerBase::handle_response(msg);
    }
};

#endif // COMMAND_LIFECYCLE_HANDLER_HPP