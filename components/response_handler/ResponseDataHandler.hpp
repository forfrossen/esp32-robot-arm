#include "../magic_enum/include/magic_enum/magic_enum.hpp"
#include "ResponseHandlerBase.hpp"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "utils.hpp"

#ifndef RESPONSE_DATA_HANDLER_HPP
#define RESPONSE_DATA_HANDLER_HPP

class ResponseDataHandler : public ResponseHandlerBase
{
public:
    ResponseDataHandler(std::shared_ptr<MotorContext> context) : ResponseHandlerBase()
    {
        this->context = context;
    }

    bool handle_response(const twai_message_t &msg) override
    {
        CommandIds command_id = static_cast<CommandIds>(msg.data[0]);
        ESP_LOGI("ResponseDataHandler", "Handling response for command ID: %d", static_cast<int>(command_id));
        extract_response_data(command_id, msg, *context);

        return ResponseHandlerBase::handle_response(msg);
    }

private:
    std::shared_ptr<MotorContext> context;
    esp_err_t extract_response_data(CommandIds command_id, const twai_message_t &msg, MotorContext &context)
    {
        esp_err_t ret = ESP_OK;

        // Find the payload info for this command ID
        auto it = ResponsePayloadMap.find(command_id);
        CHECK_THAT(it != ResponsePayloadMap.end() && it->second.has_value());

        const CommandPayloadInfo &payload_info = it->second.value();
        CHECK_THAT(payload_info.payload_types.size() > 0);

        size_t index = 1;

        ESP_LOGI(FUNCTION_NAME, "Extracting data for command ID: %d \t size of payload_types: %d", static_cast<int>(command_id), payload_info.payload_types.size());
        for (size_t i = 0; i < payload_info.payload_types.size(); ++i)
        {
            PayloadType type = payload_info.payload_types[i];
            ESP_LOGI(FUNCTION_NAME, "Extracting data for PayloadType index %zu PayloadType: %s", i, magic_enum::enum_name(type).data());
            if (type == PayloadType::VOID)
                break;

            size_t type_size = get_payload_type_size(type);
            CHECK_THAT(type_size > 0);

            int64_t value = 0;
            ESP_RETURN_ON_ERROR(extract_data_from_message(msg, index, type_size, value, true), FUNCTION_NAME, "Failed to extract data");

            std::string type_name = std::string(magic_enum::enum_name(type));
            bool is_signed = (type_name.rfind("INT", 0) == 0);

            // Perform sign extension if the type is signed
            if (is_signed)
            {
                if (type == PayloadType::INT16 && (value & 0x8000))
                {
                    value |= ~0xFFFF;
                }
                else if (type == PayloadType::INT32 && (value & 0x80000000))
                {
                    value |= ~0xFFFFFFFF;
                }
                else if (type == PayloadType::INT48 && (value & 0x800000000000))
                {
                    value |= ~0xFFFFFFFFFFFF;
                }
            }

            ESP_LOGI(FUNCTION_NAME, "Extracted value: %lld from PayloadType index %zu", value, i);

            index += type_size;
        }

        uint8_t crc_value = msg.data[msg.data_length_code - 1];
        ESP_LOGI(FUNCTION_NAME, "Extracted CRC: 0x%02X", crc_value);

        return ret;
    }

    esp_err_t extract_data_from_message(const twai_message_t &msg, size_t index, size_t size, int64_t &value, bool is_big_endian = false)
    {
        value = 0; // Initialize value to zero

        // CHECK_THAT(index + size <= msg.data_length_code);
        ESP_LOGI(FUNCTION_NAME, "index: %zu \t size: %zu \t msg.data_length_code: %d", index, size, msg.data_length_code);
        if (is_big_endian)
        {
            for (size_t i = 0; i < size; ++i)
            {
                value |= static_cast<uint64_t>(msg.data[index + i]) << (8 * (size - 1 - i));
            }
        }
        else
        {
            for (size_t i = 0; i < size; ++i)
            {
                value |= static_cast<uint64_t>(msg.data[index + i]) << (8 * i);
            }
        }

        return ESP_OK;
    }
};
#endif // COMPLEX_RESPONSE_HANDLER_HPP