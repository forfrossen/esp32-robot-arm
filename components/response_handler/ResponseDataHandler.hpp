
#ifndef RESPONSE_DATA_HANDLER_HPP
#define RESPONSE_DATA_HANDLER_HPP

#include "../magic_enum/include/magic_enum/magic_enum.hpp"

#include "MksEnums.hpp"
#include "Properties.hpp"
#include "ResponseTypeDefs.hpp"
#include "TypeDefs.hpp"
#include "lwip/def.h"

#include "ResponseHandlerBase.hpp"
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "utils.hpp"

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
        auto it = g_response_payload_map.find(command_id);
        CHECK_THAT(it != g_response_payload_map.end());

        // context->set_meta_property(&MotorProperties::last_seen, std::chrono::system_clock::now());

        return ResponseHandlerBase::handle_response(msg);
    }

private:
    std::shared_ptr<MotorContext> context;
    esp_err_t extract_response_data(CommandIds command_id, const twai_message_t &msg, MotorContext &context)
    {
        esp_err_t ret = ESP_OK;

        ESP_LOGI(FUNCTION_NAME, "Extracting response data for command ID: %d", static_cast<int>(command_id));

        const auto it = g_response_payload_map.find(command_id);
        ESP_RETURN_ON_FALSE(it != g_response_payload_map.end(), ESP_FAIL, FUNCTION_NAME, "Command ID not found in response map");
        const ResponseInformation &response_info = it->second;
        const CommandPayloadInfo &payload_info = response_info.payload_info;
        const std::vector<std::string> &property_names = response_info.property_names;

        size_t offset = 1; // Start after the CommandId byte
        for (int i = 0; i < payload_info.get_size(); ++i)
        {
            PayloadType payload_type = payload_info.get_type(i);
            size_t type_size = get_payload_type_size(payload_type);

            if (offset + type_size > msg.data_length_code)
            {
                ESP_LOGE("extract_response_data", "Insufficient data for property: %s", property_names[i].c_str());
                return ESP_FAIL;
            }

            const std::string &property_name = property_names[i];
            auto prop_meta_it = property_metadata_map.find(property_name);
            if (prop_meta_it == property_metadata_map.end())
            {
                ESP_LOGE("extract_response_data", "Property metadata not found for property: %s", property_name.c_str());
                return ESP_FAIL;
            }

            const PropertyMetadata &prop_meta = prop_meta_it->second;
            const uint8_t *data_ptr = &msg.data[offset];

            // Use the generic property setter
            set_property_value(prop_meta, data_ptr);

            offset += type_size;
        }
        return ret;
    }

    esp_err_t set_property_value(const PropertyMetadata &metadata, const uint8_t *data)
    {
        MotorProperties &properties = context->get_properties();
        uint8_t *base_ptr = reinterpret_cast<uint8_t *>(&properties);
        void *prop_ptr = base_ptr + metadata.offset;

        switch (metadata.type)
        {
        case PayloadType::UINT8:
            *reinterpret_cast<uint8_t *>(prop_ptr) = *data;
            break;

        case PayloadType::UINT16:
        {
            uint16_t value;
            memcpy(&value, data, sizeof(uint16_t));
            value = ntohs(value); // Convert from network byte order to host byte order
            *reinterpret_cast<uint16_t *>(prop_ptr) = value;
        }
        break;

        case PayloadType::UINT32:
        {
            uint32_t value;
            memcpy(&value, data, sizeof(uint32_t));
            value = ntohl(value);
            *reinterpret_cast<uint32_t *>(prop_ptr) = value;
        }
        break;

        case PayloadType::INT16:
        {
            uint16_t temp;
            memcpy(&temp, data, sizeof(uint16_t));
            temp = ntohs(temp);
            int16_t value = static_cast<int16_t>(temp);
            *reinterpret_cast<int16_t *>(prop_ptr) = value;
        }
        break;

        case PayloadType::INT32:
        {
            uint32_t temp;
            memcpy(&temp, data, sizeof(uint32_t));
            temp = ntohl(temp);
            int32_t value = static_cast<int32_t>(temp);
            *reinterpret_cast<int32_t *>(prop_ptr) = value;
        }
        break;

        case PayloadType::UINT24:
        {
            uint32_t value = extract_uint24(data);
            *reinterpret_cast<uint32_t *>(prop_ptr) = value;
        }
        break;

        case PayloadType::INT48:
        {
            int64_t value = extract_int48(data);
            *reinterpret_cast<int64_t *>(prop_ptr) = value;
        }
        break;
        default:
            ESP_LOGE("set_property_value", "Unsupported property type: %s", metadata.name.c_str());
            break;
        }

        return ESP_OK;
    }

    uint32_t extract_uint24(const uint8_t *data)
    {
        uint32_t value = 0;
        value |= static_cast<uint32_t>(data[0]) << 16;
        value |= static_cast<uint32_t>(data[1]) << 8;
        value |= static_cast<uint32_t>(data[2]);
        return value;
    }

    int64_t extract_int48(const uint8_t *data)
    {
        int64_t value = 0;
        value |= static_cast<int64_t>(data[0]) << 40;
        value |= static_cast<int64_t>(data[1]) << 32;
        value |= static_cast<int64_t>(data[2]) << 24;
        value |= static_cast<int64_t>(data[3]) << 16;
        value |= static_cast<int64_t>(data[4]) << 8;
        value |= static_cast<int64_t>(data[5]);

        // Sign extension if necessary
        if (value & 0x800000000000)
        {
            value |= ~0xFFFFFFFFFFFF; // Sign extend to 64 bits
        }

        return value;
    }

    esp_err_t
    extract_data_from_message(const twai_message_t &msg, int index, size_t size, int64_t &value, bool is_big_endian = false)
    {
        value = 0; // Initialize value to zero

        // CHECK_THAT(index + size <= msg.data_length_code);
        ESP_LOGI(FUNCTION_NAME, "index: %d \t size: %zu \t msg.data_length_code: %d", index, size, msg.data_length_code);
        if (is_big_endian)
        {
            for (int i = 0; i < size; ++i)
            {
                value |= static_cast<uint64_t>(msg.data[index]) << (8 * (size - 1 - i));
            }
        }
        else
        {
            for (int i = 0; i < size; ++i)
            {
                value |= static_cast<uint64_t>(msg.data[index]) << (8 * i);
            }
        }

        return ESP_OK;
    }
};
#endif // COMPLEX_RESPONSE_HANDLER_HPP

// esp_err_t update_motor_property(const ResponseInformation &response_info, size_t i, uint64_t raw_value)
// {
//     if (i >= response_info.mapped_properties.size())
//         return ESP_FAIL;

//     void *property_ptr = response_info.mapped_properties[i];

//     // Dynamically handle the property using `std::visit` and map raw_value to the correct type
//     std::visit([property_ptr, raw_value](auto &&val)
//                {
//     using T = std::decay_t<decltype(val)>;

//     if (auto* motor_prop = static_cast<MotorProperty<T>*>(property_ptr))
//     {
//         motor_prop->set(static_cast<T>(raw_value));  // Convert raw_value to the correct type
//     } }, MotorPropertyVariant{});

//     return ESP_OK;
// }
// Set the property using the provided variant value
// std::visit([&](auto &&val)
//            { property_ptr->set(val); }, value);
// switch (type_info)
// {
// case PayloadType::UINT4:
// case PayloadType::UINT8:
// {
//     auto property = std::any_cast<MotorProperty<uint8_t> MotorProperties::*>(mapped_property);
//     uint8_t casted_value = std::any_cast<uint8_t>(value);
//     context->set_property(property, casted_value);
//     break;
// }

// case PayloadType::UINT16:
// {
//     auto property = std::any_cast<MotorProperty<uint16_t> MotorProperties::*>(mapped_property);
//     uint16_t casted_value = std::any_cast<uint16_t>(value);
//     context->set_property(property, casted_value);
//     break;
// }

// case PayloadType::UINT24:
// case PayloadType::UINT32:
// {
//     auto property = std::any_cast<MotorProperty<uint32_t> MotorProperties::*>(mapped_property);
//     uint32_t casted_value = std::any_cast<uint32_t>(value);
//     context->set_property(property, casted_value);
//     break;
// }

// case PayloadType::UINT48:
// {
//     auto property = std::any_cast<MotorProperty<uint64_t> MotorProperties::*>(mapped_property);
//     uint64_t casted_value = std::any_cast<uint64_t>(value);
//     context->set_property(property, casted_value);
//     break;
// }

// case PayloadType::INT16:
// {
//     auto property = std::any_cast<MotorProperty<int16_t> MotorProperties::*>(mapped_property);
//     int16_t casted_value = std::any_cast<int16_t>(value);
//     context->set_property(property, casted_value);
//     break;
// }

// case PayloadType::INT24:
// case PayloadType::INT32:
// {
//     auto property = std::any_cast<MotorProperty<int32_t> MotorProperties::*>(mapped_property);
//     int32_t casted_value = std::any_cast<int32_t>(value);
//     context->set_property(property, casted_value);
//     break;
// }

// case PayloadType::INT48:
// {
//     auto property = std::any_cast<MotorProperty<int64_t> MotorProperties::*>(mapped_property);
//     int64_t casted_value = std::any_cast<int64_t>(value);
//     context->set_property(property, casted_value);
//     break;
// }

// default:
// {
//     ESP_LOGE("update_context_property", "Unsupported type for updating context property");
//     return ESP_FAIL;
// }
// }