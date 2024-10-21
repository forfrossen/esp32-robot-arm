#ifndef GENERIC_COMMAND_BUILDER_H
#define GENERIC_COMMAND_BUILDER_H

#include "Events.hpp"

#include "CommandPayloadTypeDefs.hpp"
#include "MksEnums.hpp"
#include "TypeDefs.hpp"

#include "../common/utils.hpp"

#include "CommandBase.hpp"
#include "CommandLifecycleRegistry.hpp"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/queue.h"
#include "utils.hpp"
#include <driver/twai.h>
#include <variant>
#include <vector>

class GenericCommand
{
private:
    uint32_t id;
    CommandIds command_id;
    twai_message_t msg;
    std::shared_ptr<CommandLifecycleRegistry> command_lifecycle_registry;
    SemaphoreHandle_t msg_mutex;
    esp_event_loop_handle_t system_event_loop;

public:
    bool is_error = false;
    template <typename... Args>
    GenericCommand(
        std::shared_ptr<CommandFactorySettings> settings,
        CommandIds command_id,
        Args... args) : id(settings->id),
                        command_id(command_id),
                        command_lifecycle_registry(settings->command_lifecycle_registry),
                        msg_mutex(xSemaphoreCreateMutex()),
                        system_event_loop(settings->system_event_loop)
    {
        ESP_LOGI(FUNCTION_NAME, "GenericCommand constructor called for Motor with id: %lu", id);
        esp_err_t ret = init_new_command(std::forward<Args>(args)...);
        if (ret != ESP_OK)
        {
            ESP_LOGE(FUNCTION_NAME, "Error initializing new command");
            is_error = true;
        }
    }

    ~GenericCommand()
    {
        ESP_LOGW(FUNCTION_NAME, "GenericCommand destructor called");
        vSemaphoreDelete(msg_mutex);
    }

    esp_err_t get_semaphore()
    {
        CHECK_THAT(xSemaphoreTake(msg_mutex, portMAX_DELAY) == pdTRUE);
        return ESP_OK;
    }

    template <typename... Args>
    esp_err_t init_new_command(Args... args)
    {
        CHECK_THAT(command_lifecycle_registry != nullptr);
        msg = twai_message_t();
        ESP_RETURN_ON_ERROR(set_default_values(), FUNCTION_NAME, "Error setting default values");
        CHECK_THAT(msg.identifier == id);
        ESP_LOGI(FUNCTION_NAME, "Message identifier: %lu", msg.identifier);
        ESP_RETURN_ON_ERROR(set_data(std::forward<Args>(args)...), FUNCTION_NAME, "Error setting data");
        ESP_RETURN_ON_ERROR(register_command(), FUNCTION_NAME, "Error registering command");
        return ESP_OK;
    }

    esp_err_t set_default_values()
    {
        ESP_LOGI(FUNCTION_NAME, "Setting default values");
        msg.extd = 0;
        msg.rtr = 0;
        msg.ss = 0;
        msg.self = 0;
        msg.dlc_non_comp = 0;
        msg.identifier = id;
        return ESP_OK;
    }

    esp_err_t register_command()
    {
        ESP_RETURN_ON_ERROR(
            command_lifecycle_registry->register_command(id, command_id, msg),
            FUNCTION_NAME,
            "Failed to register command for motor");

        return ESP_OK;
    }

    template <typename... Args>
    esp_err_t set_data(Args &&...args)
    {
        ESP_LOGI(FUNCTION_NAME, "Setting data for CommandId %d", static_cast<int>(command_id));

        auto it = g_command_payload_map.find(command_id);
        CHECK_THAT(it != g_command_payload_map.end());

        const CommandPayloadInfo &payload_info = it->second;
        constexpr size_t num_args = sizeof...(Args);
        size_t num_payload_types = count_valid_payloads(payload_info.type_info);
        ESP_LOGI(FUNCTION_NAME, "Number of arguments: %d", num_args);

        CHECK_THAT(num_args == num_payload_types);

        size_t index = 0;
        msg.data[index++] = static_cast<uint8_t>(command_id);

        auto args_tuple = std::make_tuple(std::forward<Args>(args)...);
        pack_arguments(index, args_tuple, payload_info, std::make_index_sequence<num_args>{});

        // Add CRC at the end
        CHECK_THAT(set_msg_data_crc(index) == ESP_OK);
        msg.data_length_code = index + 1;
        return ESP_OK;
    }

    size_t count_valid_payloads(const std::array<PayloadType, 7> &payload_types)
    {
        return std::count_if(payload_types.begin(), payload_types.end(), [](PayloadType type)
                             { return type != PayloadType::VOID; });
    }

    template <typename Tuple, std::size_t... I>
    void pack_arguments(size_t &index, Tuple &&args_tuple, const CommandPayloadInfo &payload_info, std::index_sequence<I...>)
    {

        // Expand the parameter pack to call pack_value for each argument
        ((pack_value(index, std::get<I>(args_tuple), payload_info.get_type(I))), ...);
    }

    template <typename T>
    void pack_value(size_t &index, T value, PayloadType type, bool is_big_endian = true)
    {
        if (type == PayloadType::UINT16 || type == PayloadType::INT16)
        {
            if (is_big_endian)
            {
                msg.data[index++] = static_cast<uint8_t>((value >> 8) & 0xFF);
                msg.data[index++] = static_cast<uint8_t>(value & 0xFF);
            }
            else
            {
                msg.data[index++] = static_cast<uint8_t>(value & 0xFF);
                msg.data[index++] = static_cast<uint8_t>((value >> 8) & 0xFF);
            }
        }
        else if (type == PayloadType::INT24 || type == PayloadType::UINT24)
        {
            if (is_big_endian)
            {
                msg.data[index++] = static_cast<uint8_t>((value >> 16) & 0xFF); // MSB
                msg.data[index++] = static_cast<uint8_t>((value >> 8) & 0xFF);  // Middle byte
                msg.data[index++] = static_cast<uint8_t>(value & 0xFF);         // LSB
            }
            else
            {
                msg.data[index++] = static_cast<uint8_t>(value & 0xFF);         // LSB
                msg.data[index++] = static_cast<uint8_t>((value >> 8) & 0xFF);  // Middle byte
                msg.data[index++] = static_cast<uint8_t>((value >> 16) & 0xFF); // MSB
            }
        }
        else if (type == PayloadType::UINT8)
        {
            msg.data[index++] = static_cast<uint8_t>(value);
        }
        else
        {
            ESP_LOGE(FUNCTION_NAME, "Unsupported type for packing");
        }
    }

    esp_err_t set_msg_data_crc(size_t &index)
    {
        ESP_LOGI(FUNCTION_NAME, "Setting message data CRC");
        ESP_LOGI(FUNCTION_NAME, "Index: %d", index);
        CHECK_THAT(index != 0);
        uint8_t crc = 0;
        esp_err_t ret = calculate_crc(crc, index);
        if (crc == 0 || ret != ESP_OK)
        {
            ESP_LOGE(FUNCTION_NAME, "CRC calculation failed");
            return ESP_FAIL;
        }
        msg.data[index] = crc;
        ESP_LOGI(FUNCTION_NAME, "CRC: 0x%02X", msg.data[index]);
        CHECK_THAT(msg.data[index] != 0);
        return ESP_OK;
    }

    esp_err_t calculate_crc(uint8_t &crc, size_t &index)
    {
        ESP_LOGI(FUNCTION_NAME, "Calculating CRC");
        ESP_LOGI(FUNCTION_NAME, "Index: %d", index);
        ESP_LOGI(FUNCTION_NAME, "Command code: 0x%02X", msg.data[0]);
        ESP_LOGI(FUNCTION_NAME, "Identifier: %lu", msg.identifier);
        if (msg.identifier == 0)
        {
            ESP_LOGE(FUNCTION_NAME, "identifier is 0");
            msg.identifier = id;
        }

        CHECK_THAT(msg.identifier > 0);
        CHECK_THAT(index > 0);
        crc = msg.identifier;
        for (uint8_t i = 0; i < index; i++)
        {
            crc += msg.data[i];
        }
        crc &= 0xFF;
        return ESP_OK;
    }

    esp_err_t execute()
    {
        ESP_RETURN_ON_ERROR(get_semaphore(), FUNCTION_NAME, "Failed to take mutex");
        ESP_LOGI(FUNCTION_NAME, "Copying data to aligned message");
        twai_message_t aligned_msg = msg;
        xSemaphoreGive(msg_mutex);
        ESP_LOGI(FUNCTION_NAME, "Posting event");
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
#endif // GENERIC_COMMAND_BUILDER_H