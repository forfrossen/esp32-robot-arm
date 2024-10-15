#include "ResponseHandlerBase.hpp"
#include "esp_log.h"
#include "utils.hpp"

#ifndef COMPLEX_RESPONSE_HANDLER_HPP
#define COMPLEX_RESPONSE_HANDLER_HPP

struct ResponsePayloadInfo
{
    CommandIds command_id;

    ResponsePayloadInfo(CommandIds command_id) : command_id(command_id) {}

    template <typename... Args>
    esp_err_t define_payload(Args... args)
    {
        // Ensure that all arguments are of valid types (uint8_t, uint16_t, or uint32_t)
        static_assert((... && (std::is_same_v<Args, uint8_t> ||
                               std::is_same_v<Args, uint16_t> ||
                               std::is_same_v<Args, uint24_t> ||
                               std::is_same_v<Args, uint32_t>)),
                      "Arguments must be of type uint8_t, uint16_t, uint24_t or uint32_t");

        size_t index = 0;

        // Lambda to handle each argument type and fill the array
        ([&]
         {
            if constexpr (std::is_same_v<Args, uint8_t>) {
                if (index < data_length) data[index++] = args;  // Directly access the C-style array
            }
            else if constexpr (std::is_same_v<Args, uint16_t>) {
                if (index < data_length) data[index++] = static_cast<uint8_t>(args & 0xFF);         // Lower byte
                if (index < data_length) data[index++] = static_cast<uint8_t>((args >> 8) & 0xFF);  // Upper byte
            }
            else if constexpr (std::is_same_v<Args, uint24_t>) {
                if (index < data_length) data[index++] = static_cast<uint8_t>(args & 0xFF);  // Byte 1 (lowest)
                if (index < data_length) data[index++] = static_cast<uint8_t>((args >> 8) & 0xFF);  // Byte 2 (middle)
                if (index < data_length) data[index++] = static_cast<uint8_t>((args >> 16) & 0xFF);  // Byte 3 (highest
            }
            else if constexpr (std::is_same_v<Args, uint32_t>) {
                if (index < data_length) data[index++] = static_cast<uint8_t>(args & 0xFF);         // Byte 1
                if (index < data_length) data[index++] = static_cast<uint8_t>((args >> 8) & 0xFF);  // Byte 2
                if (index < data_length) data[index++] = static_cast<uint8_t>((args >> 16) & 0xFF);  // Byte 3
                if (index < data_length) data[index++] = static_cast<uint8_t>((args >> 24) & 0xFF);  // Byte 4
            } }(), ...);
        return ESP_OK;
    }
};

class ComplexResponseHandler : public ResponseHandlerBase
{
public:
    ComplexResponseHandler(std::shared_ptr<MotorContext> context) : ResponseHandlerBase()
    {
        this->context = context;
    }

    bool handle_response(const twai_message_t &msg) override
    {
        CommandIds command_id = static_cast<CommandIds>(msg.data[0]);
        // ResponsePayloadInfo response_payload_info = new
        // No error, pass to the next handler
        return ResponseHandlerBase::handle_response(msg);
    }

private:
    std::shared_ptr<MotorContext> context;
};
#endif // COMPLEX_RESPONSE_HANDLER_HPP