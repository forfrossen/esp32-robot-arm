#ifndef TWAI_COMMAND_BUILDER_BASE_HPP
#define TWAI_COMMAND_BUILDER_BASE_HPP

#include "Events.hpp"
#include "TypeDefs.hpp"
#include "utils.hpp"
#include <driver/twai.h>
#include <esp_check.h>
#include <esp_err.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <vector>

class CommandLifecycleRegistry;
template <typename T>
class CommandBase
{
protected:
    uint32_t id;
    motor_command_id_t command_id;
    uint8_t cmd_code;
    uint8_t data_length = 0;
    std::optional<twai_message_t> msg;
    uint8_t *data = nullptr;
    esp_event_loop_handle_t system_event_loop;
    std::shared_ptr<CommandLifecycleRegistry> command_lifecycle_registry;
    SemaphoreHandle_t msg_mutex;
    std::optional<CommandPayloadInfo> payload_info;

public:
    CommandBase(std::shared_ptr<CommandFactorySettings> settings, motor_command_id_t command_id) : id(settings->id), command_id(command_id), system_event_loop(settings->system_event_loop), command_lifecycle_registry(settings->command_lifecycle_registry), msg_mutex(xSemaphoreCreateMutex())
    {
        uint8_t command_id_int = static_cast<uint8_t>(command_id);

        ESP_LOGD(FUNCTION_NAME, "CommandBase constructor called for command_id: 0x%02X", command_id_int);
        esp_err_t ret = init_new_command(command_id);
        if (ret != ESP_OK)
        {
            ESP_LOGE(FUNCTION_NAME, "Error initializing new command");
        }
        else
        {
            ESP_LOGD(FUNCTION_NAME, "New command initialized successfully");
        }
    }

    esp_err_t init_new_command(motor_command_id_t command_id)
    {
        CHECK_THAT(command_lifecycle_registry != nullptr);
        msg = twai_message_t();
        ESP_RETURN_ON_ERROR(set_default_values(), FUNCTION_NAME, "Error setting default values");
        ESP_RETURN_ON_ERROR(set_command_id(command_id), FUNCTION_NAME, "Error setting command code");
        ESP_RETURN_ON_ERROR(set_payload_info(), FUNCTION_NAME, "Error setting payload info");
        ESP_RETURN_ON_ERROR(calculate_payload_size(), FUNCTION_NAME, "Error calculating payload size");
        ESP_RETURN_ON_ERROR(set_data_length_code(), FUNCTION_NAME, "Error setting data length code");
        ESP_RETURN_ON_ERROR(create_msg_data(), FUNCTION_NAME, "Error creating message data");
        ESP_RETURN_ON_ERROR(register_command(), FUNCTION_NAME, "Error registering command");
        return ESP_OK;
    }

    esp_err_t msg_check()
    {
        CHECK_THAT(msg.has_value());
        return ESP_OK;
    }

    esp_err_t data_check()
    {
        CHECK_THAT(data != nullptr);
        return ESP_OK;
    }

    esp_err_t get_semaphore()
    {
        CHECK_THAT(xSemaphoreTake(msg_mutex, portMAX_DELAY) == pdTRUE);
        return ESP_OK;
    }

    virtual ~CommandBase() = default;

    esp_err_t set_default_values()
    {
        ESP_RETURN_ON_ERROR(msg_check(), FUNCTION_NAME, "msg is nullptr");
        ESP_LOGD(FUNCTION_NAME, "Setting default values");
        msg.value().extd = 0;
        msg.value().rtr = 0;
        msg.value().ss = 0;
        msg.value().self = 0;
        msg.value().dlc_non_comp = 0;
        msg.value().identifier = id;
        return ESP_OK;
    }

    esp_err_t set_command_id(motor_command_id_t command_id)
    {
        ESP_RETURN_ON_ERROR(msg_check(), FUNCTION_NAME, "msg is nullptr");
        cmd_code = static_cast<uint8_t>(command_id);
        ESP_LOGD(FUNCTION_NAME, "Setting command code to: 0x%02X", cmd_code);
        CHECK_THAT(cmd_code == static_cast<uint8_t>(command_id));
        // ESP_LOGD(FUNCTION_NAME, "Command code: 0x%02X", command_id);
        // memset(&msg, 0, sizeof(msg));
        // ESP_LOGD(FUNCTION_NAME, "Command msg has been zeroed out");
        return ESP_OK;
    }

    esp_err_t set_payload_info()
    {
        ESP_RETURN_ON_ERROR(get_semaphore(), FUNCTION_NAME, "Failed to take mutex");
        auto it = g_command_payload_map.find(command_id);
        xSemaphoreGive(msg_mutex);
        CHECK_THAT(it != g_command_payload_map.end());
        payload_info.emplace(it->second);
        return ESP_OK;
    }

    esp_err_t calculate_payload_size()
    {
        ESP_LOGD(FUNCTION_NAME, "Calculating payload size");
        CHECK_THAT(payload_info.has_value());
        ESP_RETURN_ON_ERROR(get_semaphore(), FUNCTION_NAME, "Failed to take mutex");
        for (const auto &type : payload_info.value().type_info)
        {
            switch (type)
            {
            case PayloadType::UINT8:
                data_length += sizeof(uint8_t);
                break;
            case PayloadType::UINT16:
                data_length += sizeof(uint16_t);
                break;
            case PayloadType::UINT24:
                data_length += sizeof(uint16_t) + sizeof(uint8_t);
                break;
            case PayloadType::UINT32:
                data_length += sizeof(uint32_t);
                break;
            case PayloadType::VOID:
                // No size to add for VOID
                break;
            default:
                ESP_LOGE(FUNCTION_NAME, "Unsupported payload type");
            }
        }
        // Add 2 bytes for the command code and the crc
        data_length += 2;
        xSemaphoreGive(msg_mutex);
        ESP_LOGD(FUNCTION_NAME, "data_length size: %d", data_length);
        CHECK_THAT(data_length <= 8);
        return ESP_OK;
    }

    esp_err_t register_command()
    {
        // command_lifecycle_registry->register_command(id, command_id);
        return ESP_OK;
    }

    esp_err_t set_data_length_code()
    {
        ESP_RETURN_ON_ERROR(msg_check(), FUNCTION_NAME, "msg is nullptr");
        ESP_RETURN_ON_ERROR(get_semaphore(), FUNCTION_NAME, "Failed to take mutex");
        ESP_LOGD(FUNCTION_NAME, "Setting data length code to: %d", data_length);
        msg.value().data_length_code = data_length;
        xSemaphoreGive(msg_mutex);
        CHECK_THAT(msg.value().data_length_code == data_length);
        return ESP_OK;
    }

    esp_err_t create_msg_data()
    {
        ESP_LOGD(FUNCTION_NAME, "Creating message data");
        data = new uint8_t[msg.value().data_length_code];
        return ESP_OK;
    }

    virtual esp_err_t build_twai_message() = 0;

    esp_err_t execute()
    {
        ESP_LOGD(FUNCTION_NAME, "Building and sending message");
        ESP_RETURN_ON_ERROR(msg_check(), FUNCTION_NAME, "msg is nullptr");
        ESP_RETURN_ON_ERROR(build_twai_message(), FUNCTION_NAME, "Error building TWAI message");
        ESP_RETURN_ON_ERROR(set_msg_data(), FUNCTION_NAME, "Error setting message data");
        ESP_RETURN_ON_ERROR(set_msg_data_crc(), FUNCTION_NAME, "Error setting message data CRC");
        ESP_RETURN_ON_ERROR(post_event(), FUNCTION_NAME, "Error posting event");

        return ESP_OK;
    }

    esp_err_t set_msg_data()
    {
        ESP_LOGD(FUNCTION_NAME, "Setting message data");
        ESP_RETURN_ON_ERROR(msg_check(), FUNCTION_NAME, "msg is nullptr");
        ESP_RETURN_ON_ERROR(data_check(), FUNCTION_NAME, "data is nullptr");
        CHECK_THAT(msg.value().data_length_code <= sizeof(msg.value().data));
        ESP_RETURN_ON_ERROR(get_semaphore(), FUNCTION_NAME, "Failed to take mutex");
        memcpy(msg.value().data, data, msg.value().data_length_code);
        xSemaphoreGive(msg_mutex);
        return ESP_OK;
    }

    esp_err_t set_msg_data_crc()
    {
        ESP_RETURN_ON_ERROR(msg_check(), FUNCTION_NAME, "msg is nullptr");
        ESP_LOGD(FUNCTION_NAME, "Setting message data CRC");
        CHECK_THAT(msg.value().data_length_code != 0);
        ESP_RETURN_ON_ERROR(get_semaphore(), FUNCTION_NAME, "Failed to take mutex");
        uint8_t crc = 0;
        esp_err_t ret = calculate_crc(crc);
        if (crc == 0 || ret != ESP_OK)
        {
            xSemaphoreGive(msg_mutex);
            ESP_LOGE(FUNCTION_NAME, "CRC calculation failed");
            return ESP_FAIL;
        }
        msg.value().data[msg.value().data_length_code - 1] = crc;
        xSemaphoreGive(msg_mutex);
        CHECK_THAT(msg.value().data[msg.value().data_length_code - 1] != 0);
        ESP_LOGD(FUNCTION_NAME, "CRC: 0x%02X", msg.value().data[msg.value().data_length_code - 1]);
        return ESP_OK;
    }

    esp_err_t calculate_crc(uint8_t &crc)
    {
        ESP_LOGD(FUNCTION_NAME, "Calculating CRC");
        CHECK_THAT(msg.value().data_length_code != 0);
        crc = msg.value().identifier;
        for (uint8_t i = 0; i < msg.value().data_length_code - 1; i++)
        {
            crc += msg.value().data[i];
        }
        crc &= 0xFF;
        return ESP_OK;
    }

    esp_err_t post_event()
    {
        ESP_RETURN_ON_ERROR(get_semaphore(), FUNCTION_NAME, "Failed to take mutex");
        ESP_LOGD(FUNCTION_NAME, "Copying data to aligned message");
        twai_message_t aligned_msg = msg.value();
        xSemaphoreGive(msg_mutex);
        ESP_LOGD(FUNCTION_NAME, "Posting event");
        CHECK_THAT(system_event_loop != nullptr);

        esp_err_t ret = esp_event_post_to(
            system_event_loop,
            SYSTEM_EVENTS,
            OUTGOING_MESSAGE_EVENT,
            &aligned_msg,
            sizeof(twai_message_t),
            portMAX_DELAY);

        ESP_RETURN_ON_ERROR(
            ret,
            FUNCTION_NAME,
            "Error posting event");
        return ESP_OK;
    }
};

#endif // TWAI_COMMAND_BUILDER_BASE_HPP
