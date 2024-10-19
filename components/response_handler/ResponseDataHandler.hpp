
#ifndef RESPONSE_DATA_HANDLER_HPP
#define RESPONSE_DATA_HANDLER_HPP

#include "../magic_enum/include/magic_enum/magic_enum.hpp"

#include "MksEnums.hpp"
#include "ResponseTypeDefs.hpp"
#include "TypeDefs.hpp"

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

        // Find the payload info for this command ID
        const auto it = g_response_payload_map.find(command_id);
        CHECK_THAT(it != g_response_payload_map.end());

        const ResponseInformation &response_info = it->second;

        CHECK_THAT(response_info.payload_info.get_size() > 0);
        const auto payload_info = response_info.payload_info;
        const auto mapped_properties = response_info.mapped_properties;

        ESP_LOGI(FUNCTION_NAME, "Extracting data for command: %s \t size of payload_types: %d", get_command_name(&msg), payload_info.get_size());
        for (size_t i = 0; i < payload_info.get_size(); ++i)
        {
            PayloadType type_info = payload_info.type_info[i];
            ESP_LOGI(FUNCTION_NAME, "Extracting data for PayloadType index %zu PayloadType: %s", i, magic_enum::enum_name(type_info).data());
            // CHECK_THAT(!mapped_properties[i].has_value());

            if (type_info == PayloadType::VOID)
                break;

            size_t type_size = get_payload_type_size(type_info);
            CHECK_THAT(type_size > 0);

            int64_t value = 0;
            ESP_RETURN_ON_ERROR(extract_data_from_message(msg, i, type_size, value, true), FUNCTION_NAME, "Failed to extract data");

            std::string type_name = std::string(magic_enum::enum_name(type_info));
            bool is_signed = (type_name.rfind("INT", 0) == 0);

            // Perform sign extension if the type is signed
            if (is_signed)
            {
                if (type_info == PayloadType::INT16 && (value & 0x8000))
                {
                    value |= ~0xFFFF;
                }
                else if (type_info == PayloadType::INT32 && (value & 0x80000000))
                {
                    value |= ~0xFFFFFFFF;
                }
                else if (type_info == PayloadType::INT48 && (value & 0x800000000000))
                {
                    value |= ~0xFFFFFFFFFFFF;
                }
            }

            ESP_LOGI(FUNCTION_NAME, "Extracted value: %lld from PayloadType[i] %zu", value, i);
            // Check that the property index is valid
            CHECK_THAT(i < response_info.get_num_properties());

            // Get the property mapped to this index
            auto property_ptr = response_info.mapped_properties[i];

            // Switch case for setting primitive types and enum types
            switch (type_info)
            {
            // Primitive types
            case PayloadType::UINT8:
                if (auto *motor_prop = std::any_cast<MotorProperty<Direction> *>(property_ptr))
                {
                    motor_prop->set(static_cast<Direction>(value));
                }
                else if (auto *motor_prop = std::any_cast<MotorProperty<Enable> *>(property_ptr))
                {
                    motor_prop->set(static_cast<Enable>(value));
                }
                else if (auto *motor_prop = std::any_cast<MotorProperty<EnableStatus> *>(property_ptr))
                {
                    motor_prop->set(static_cast<EnableStatus>(value));
                }
                else if (auto *motor_prop = std::any_cast<MotorProperty<MotorStatus> *>(property_ptr))
                {
                    motor_prop->set(static_cast<MotorStatus>(value));
                }
                else if (auto *motor_prop = std::any_cast<MotorProperty<RunMotorResult> *>(property_ptr))
                {
                    motor_prop->set(static_cast<RunMotorResult>(value));
                }
                else if (auto *motor_prop = std::any_cast<MotorProperty<MotorShaftProtectionStatus> *>(property_ptr))
                {
                    motor_prop->set(static_cast<MotorShaftProtectionStatus>(value));
                }
                else if (auto *motor_prop = std::any_cast<MotorProperty<Mode0> *>(property_ptr))
                {
                    motor_prop->set(static_cast<Mode0>(value));
                }
                else if (auto *motor_prop = std::any_cast<MotorProperty<SaveCleanState> *>(property_ptr))
                {
                    motor_prop->set(static_cast<SaveCleanState>(value));
                }
                else if (auto *motor_prop = std::any_cast<MotorProperty<CalibrationResult> *>(property_ptr))
                {
                    motor_prop->set(static_cast<CalibrationResult>(value));
                }
                else if (auto *motor_prop = std::any_cast<MotorProperty<EndStopLevel> *>(property_ptr))
                {
                    motor_prop->set(static_cast<EndStopLevel>(value));
                }
                else if (auto *motor_prop = std::any_cast<MotorProperty<CanBitrate> *>(property_ptr))
                {
                    motor_prop->set(static_cast<CanBitrate>(value));
                }
                else if (auto *motor_prop = std::any_cast<MotorProperty<uint8_t> *>(property_ptr))
                {
                    motor_prop->set(static_cast<uint8_t>(value));
                }
                break;

            case PayloadType::UINT16:

                if (auto *motor_prop = std::any_cast<MotorProperty<uint16_t> *>(property_ptr))
                {
                    motor_prop->set(static_cast<uint16_t>(value));
                }
                break;

            case PayloadType::UINT32:
                if (auto *motor_prop = std::any_cast<MotorProperty<uint32_t> *>(property_ptr))
                {
                    motor_prop->set(static_cast<uint32_t>(value));
                }
                break;

            case PayloadType::INT16:
                if (auto *motor_prop = std::any_cast<MotorProperty<int16_t> *>(property_ptr))
                {
                    motor_prop->set(static_cast<int16_t>(value));
                }
                break;

            case PayloadType::INT32:
                if (auto *motor_prop = std::any_cast<MotorProperty<int32_t> *>(property_ptr))
                {
                    motor_prop->set(static_cast<int32_t>(value));
                }
                break;

            default:
                ESP_LOGE(FUNCTION_NAME, "Unsupported PayloadType for property setting");
                return ESP_FAIL;
            }

            uint8_t crc_value = msg.data[msg.data_length_code - 1];
            ESP_LOGI(FUNCTION_NAME, "Extracted CRC: 0x%02X", crc_value);
        }

        return ret;
    }

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

    esp_err_t
    extract_data_from_message(const twai_message_t &msg, size_t index, size_t size, int64_t &value, bool is_big_endian = false)
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