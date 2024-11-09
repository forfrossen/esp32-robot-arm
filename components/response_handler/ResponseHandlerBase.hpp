
#ifndef RESPONSE_HANDLER_BASE_HPP
#define RESPONSE_HANDLER_BASE_HPP

#include "Context.hpp"
#include <driver/twai.h> // Include the header file for twai_message_t
#include <memory>

class ResponseHandlerBase
{
protected:
    std::shared_ptr<ResponseHandlerBase> next_handler;
    MotorContext *context;

public:
    virtual ~ResponseHandlerBase() = default;

    // Set the next handler in the chain
    void set_next(std::shared_ptr<ResponseHandlerBase> handler)
    {
        next_handler = handler;
    }

    // Abstract method to handle the response
    virtual bool handle_response(const twai_message_t &msg)
    {
        if (next_handler)
        {
            return next_handler->handle_response(msg);
        }
        return false;
    }
};

#endif // RESPONSE_HANDLER_BASE_HPP