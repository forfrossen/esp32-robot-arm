#include "Utilities.hpp"

static const char *TAG = "Utilities";

esp_err_t parse_json_msg(const std::string &message, ws_message_t &msg)
{
    cJSON *json = cJSON_Parse(message.c_str());
    if (!json)
    {
        ESP_LOGE(TAG, "Failed to parse JSON message");
        return ESP_ERR_INVALID_ARG;
    }

    cJSON *command_name = cJSON_GetObjectItemCaseSensitive(json, "command");
    cJSON *payload = cJSON_GetObjectItemCaseSensitive(json, "payload");

    if (!cJSON_IsString(command_name) || !cJSON_IsString(payload))
    {
        ESP_LOGE(TAG, "Invalid JSON format: missing command or payload");
        cJSON_Delete(json);
        return ESP_ERR_INVALID_ARG;
    }
    const char *command_name_string = command_name->valuestring;
    ESP_LOGD(TAG, "Command: %s", command_name_string);
    ws_command_id cmd = magic_enum::enum_cast<ws_command_id>(command_name_string).value_or(ws_command_id::UNKNOWN);
    if (cmd == ws_command_id::UNKNOWN)
    {
        ESP_LOGE(TAG, "Invalid command: %s", command_name->valuestring);
        cJSON_Delete(json);
        return ESP_ERR_INVALID_ARG;
    }

    msg.first = cmd;
    msg.second = payload->valuestring;

    cJSON_Delete(json);
    return ESP_OK;
}

esp_err_t payload_is_valid(ws_payload_t &payload)
{
    if (std::holds_alternative<int>(payload))
    {
        ESP_LOGD(TAG, "Payload is an integer");
        return ESP_OK;
    }
    else if (std::holds_alternative<std::string>(payload))
    {
        ESP_LOGD(TAG, "Payload is a string");
        return ESP_OK;
    }
    else if (std::holds_alternative<RunMode>(payload))
    {
        ESP_LOGD(TAG, "Payload is a RunMode");
        return ESP_OK;
    }
    else
    {
        ESP_LOGE(TAG, "Invalid payload type");
        return ESP_ERR_INVALID_ARG;
    }
}

esp_err_t get_run_mode(ws_payload_t &payload, RunMode &run_mode)
{
    CHECK_THAT(payload_is_valid(payload) == ESP_OK);

    if (std::holds_alternative<RunMode>(payload))
    {
        run_mode = std::get<RunMode>(payload);
        return ESP_OK;
    }
    else if (std::holds_alternative<int>(payload))
    {
        run_mode = static_cast<RunMode>(std::get<int>(payload));
        return ESP_OK;
    }
    else if (std::holds_alternative<std::string>(payload))
    {
        std::string run_mode_str = "RUNMODE" + std::get<std::string>(payload);
        run_mode = magic_enum::enum_cast<RunMode>(run_mode_str).value_or(RunMode::UNKNOWN);
        ESP_LOGD(TAG, "run_mode_str: %s - run mode: %s", run_mode_str.c_str(), magic_enum::enum_name(run_mode).data());
        if (run_mode == RunMode::UNKNOWN)
        {
            ESP_LOGE(TAG, "Invalid run mode: %s", run_mode_str.c_str());
            return ESP_ERR_INVALID_ARG;
        }
        return ESP_OK;
    }
    else
    {
        ESP_LOGE(TAG, "Invalid payload type");
        return ESP_ERR_INVALID_ARG;
    }
}