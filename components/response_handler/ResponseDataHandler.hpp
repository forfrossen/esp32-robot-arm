
#ifndef RESPONSE_DATA_HANDLER_HPP
#define RESPONSE_DATA_HANDLER_HPP

#include <magic_enum.hpp>

#include "DataExtractor.hpp"
#include "MksEnums.hpp"
#include "Properties.hpp"
#include "ResponseTypeDefs.hpp"
#include "TypeDefs.hpp"
#include "TypeTraits.hpp"
#include "lwip/def.h"

#include "ResponseHandlerBase.hpp"
#include "utils.hpp"
#include <esp_check.h>
#include <esp_err.h>
#include <esp_log.h>

class ResponseDataHandler : public ResponseHandlerBase
{
public:
    ResponseDataHandler(std::shared_ptr<MotorContext> context) : ResponseHandlerBase()
    {
        this->context = context;
    }

    bool handle_response(const twai_message_t &msg) override
    {
        motor_command_id_t command_id = static_cast<motor_command_id_t>(msg.data[0]);
        ESP_LOGD("ResponseDataHandler", "Handling response for command ID: %d", static_cast<int>(command_id));
        ESP_RETURN_ON_ERROR(extract_response_data(command_id, msg, *context), FUNCTION_NAME, "Failed to extract response data");
        auto it = g_response_payload_map.find(command_id);
        CHECK_THAT(it != g_response_payload_map.end());

        // context->set_meta_property(&MotorProperties::last_seen, std::chrono::system_clock::now());

        return ResponseHandlerBase::handle_response(msg);
    }

private:
    std::shared_ptr<MotorContext> context;
    esp_err_t extract_response_data(motor_command_id_t command_id, const twai_message_t &msg, MotorContext &context)
    {
        esp_err_t ret = ESP_OK;

        ESP_LOGD(FUNCTION_NAME, "Extracting response data for command ID: %d", static_cast<int>(command_id));

        const auto it = g_response_payload_map.find(command_id);
        ESP_RETURN_ON_FALSE(it != g_response_payload_map.end(), ESP_FAIL, FUNCTION_NAME, "Command ID not found in response map");
        const ResponseInformation &response_info = it->second;
        const CommandPayloadInfo &payload_info = response_info.payload_info;
        const std::vector<std::string> &property_names = response_info.property_names;

        size_t bit_offset = 8;

        for (int i = 0; i < payload_info.get_size(); ++i)
        {
            PayloadType payload_type = payload_info.get_type(i);
            size_t bit_length = get_payload_type_size(payload_type);
            size_t total_bits = msg.data_length_code * 8;

            ESP_RETURN_ON_FALSE(
                bit_offset + bit_length <= total_bits,
                ESP_FAIL,
                FUNCTION_NAME,
                "Insufficient data for property: %s | bit_offset: %d | bit_length: %d | payload_type_size: %d | data_length_code: %d",
                property_names[i].c_str(), bit_offset, bit_length, get_payload_type_size(payload_type), msg.data_length_code * 8);

            const std::string &property_name = property_names[i];
            auto prop_meta_it = response_property_metadata_map.find(property_name);

            ESP_RETURN_ON_FALSE(
                prop_meta_it != response_property_metadata_map.end(),
                ESP_FAIL,
                FUNCTION_NAME,
                "Property metadata not found %s",
                property_name.c_str());

            const ResponsePropertyMetadata &metadata = prop_meta_it->second;

            uint64_t raw_value = extract_data(payload_type, msg.data, bit_offset);
            ESP_LOGD(FUNCTION_NAME, "Extracted value: %llu for property: %s", raw_value, property_name.c_str());
            MotorProperties &properties = context.get_properties();
            uint8_t *base_ptr = reinterpret_cast<uint8_t *>(&properties);
            void *prop_ptr = base_ptr + metadata.offset;
            bool value_changed = metadata.setter(prop_ptr, raw_value);
            // metadata.last_seen = std::chrono::system_clock::now();

            if (value_changed)
            {
                ESP_LOGD(FUNCTION_NAME, "===============================================================");
                ESP_LOGD(FUNCTION_NAME, "Property changed: %s", property_name.c_str());
                ESP_LOGD(FUNCTION_NAME, "Property %s changed to value: %llu", property_name.c_str(), raw_value);
                ESP_LOGD(FUNCTION_NAME, "===============================================================");
                // context.post_property_change_event(metadata.name, prop_ptr, metadata.type);
            }

            bit_offset += bit_length;
        }
        return ret;
    }
};

#endif // RESPONSE_DATA_HANDLER_HPP