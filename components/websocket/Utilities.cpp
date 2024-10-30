#include "Utilities.hpp"
#include "cJSON.h"
#include "esp_log.h"

static const char *TAG = "Utilities";

esp_err_t parse_json_msg(const std::string &message, ws_message_t &msg)
{
    cJSON *json = cJSON_Parse(message.c_str());
    if (!json)
    {
        ESP_LOGE(TAG, "Failed to parse JSON message");
        return ESP_ERR_INVALID_ARG;
    }

    cJSON *command = cJSON_GetObjectItemCaseSensitive(json, "command");
    cJSON *payload = cJSON_GetObjectItemCaseSensitive(json, "payload");

    if (!cJSON_IsString(command) || !cJSON_IsString(payload))
    {
        ESP_LOGE(TAG, "Invalid JSON format: missing command or payload");
        cJSON_Delete(json);
        return ESP_ERR_INVALID_ARG;
    }

    msg.first = command->valuestring;
    msg.second = payload->valuestring;

    cJSON_Delete(json);
    return ESP_OK;
}
