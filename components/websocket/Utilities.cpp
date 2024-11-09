#include "Utilities.hpp"

static const char *TAG = "Utilities";

esp_err_t parse_json_msg(const std::string &message, ws_message_t &msg)
{
    // Versuch, die Nachricht als JSON zu parsen
    nlohmann::json json;
    ESP_RETURN_ON_FALSE(!message.empty(), ESP_ERR_INVALID_ARG, TAG, "Empty message");
    ESP_RETURN_ON_FALSE(nlohmann::json::accept(message), ESP_ERR_INVALID_ARG, TAG, "Failed to parse JSON message");

    json = nlohmann::json::parse(message, nullptr, false);
    ESP_RETURN_ON_FALSE(!json.is_discarded(), ESP_ERR_INVALID_ARG, TAG, "Failed to parse JSON message");
    ESP_RETURN_ON_FALSE(json.contains("command"), ESP_ERR_INVALID_ARG, TAG, "Invalid JSON format: missing command");
    ESP_RETURN_ON_FALSE(json.contains("payload"), ESP_ERR_INVALID_ARG, TAG, "Invalid JSON format: missing payload");

    std::string command_name_string = json["command"].get<std::string>();
    ESP_LOGD(TAG, "Command: %s", command_name_string.c_str());

    system_command_id_t cmd = magic_enum::enum_cast<system_command_id_t>(command_name_string).value_or(system_command_id_t::UNKNOWN);
    if (cmd == system_command_id_t::UNKNOWN)
    {
        ESP_LOGE(TAG, "Invalid command: %s", command_name_string.c_str());
        return ESP_ERR_INVALID_ARG;
    }

    // Setze die Felder in der Nachricht
    msg.command = cmd;
    msg.params = json["params"].get<std::string>();

    return ESP_OK;
}

esp_err_t get_run_level_from_json(json &params, RunLevel &run_level)
{
    ESP_LOGD(TAG, "Parsing run level from JSON. parms: %s", params.dump().c_str());
    int rnlvl = params["runlevel"].get<int>();
    std::string rls = std::to_string(rnlvl);
    ESP_LOGD(TAG, "Run level: %s", rls.c_str());
    run_level = magic_enum::enum_cast<RunLevel>("RUNLEVEL" + rls).value_or(RunLevel::UNKNOWN);
    ESP_LOGD(TAG, "Run level: %s", magic_enum::enum_name(run_level).data());
    if (run_level == RunLevel::UNKNOWN)
    {
        ESP_LOGE(TAG, "Invalid run level: %d", rnlvl);
        return ESP_ERR_INVALID_ARG;
    }
    ESP_LOGD(TAG, "Run level: %s", magic_enum::enum_name(run_level).data());
    ESP_RETURN_ON_FALSE(run_level != RunLevel::UNKNOWN, ESP_ERR_INVALID_ARG, TAG, "Invalid run mode");
    return ESP_OK;
}

std::string generate_uuid()
{
    uint8_t uuid[16];
    for (int i = 0; i < 16; ++i)
    {
        uuid[i] = esp_random() & 0xFF;
    }

    std::stringstream ss;
    for (int i = 0; i < 16; ++i)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)uuid[i];
        if (i == 3 || i == 5 || i == 7 || i == 9)
            ss << "-";
    }
    return ss.str();
}

esp_err_t get_cookie_value(const char *cookie_header, const std::string &key, std::string &cookie_value)
{
    std::string cookies(cookie_header);

    size_t pos = cookies.find(key + "=");
    ESP_RETURN_ON_FALSE(pos != std::string::npos, ESP_ERR_INVALID_ARG, TAG, "Cookie not found");

    pos += key.length() + 1;
    size_t end = cookies.find(';', pos);
    ESP_RETURN_ON_FALSE(end != std::string::npos, ESP_ERR_INVALID_ARG, TAG, "Invalid cookie format");

    end = cookies.length();
    cookie_value = cookies.substr(pos, end - pos);

    return ESP_OK;
}

esp_err_t receive_frame(httpd_req_t *req, httpd_ws_frame_t &ws_pkt, uint8_t *&buf)
{
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    ESP_RETURN_ON_FALSE(req != nullptr, ESP_ERR_INVALID_ARG, TAG, "Request is null");
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    ESP_RETURN_ON_ERROR(ret, TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
    ESP_LOGD(TAG, "Frame length is %d", ws_pkt.len);
    ESP_RETURN_ON_FALSE(ws_pkt.len > 0, ESP_ERR_INVALID_ARG, TAG, "Frame length is 0");

    buf = (uint8_t *)calloc(1, ws_pkt.len + 1); // +1 for NULL termination

    ESP_RETURN_ON_FALSE(buf != nullptr, ESP_ERR_NO_MEM, TAG, "Failed to allocate memory for buffer");

    ws_pkt.payload = buf;

    // Receive actual payload
    ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
        free(buf);
        return ret;
    }

    ESP_LOGD(TAG, "Received packet with message: %s", ws_pkt.payload);
    return ESP_OK;
}

esp_err_t parse_json_rpc(const std::string &incoming_message, ws_message_t &msg)
{

    ESP_RETURN_ON_FALSE(!incoming_message.empty(), ESP_ERR_INVALID_ARG, TAG, "Empty message");
    ESP_RETURN_ON_FALSE(nlohmann::json::accept(incoming_message), ESP_ERR_INVALID_ARG, TAG, "Failed to parse JSON message");

    try
    {
        json message = json::parse(incoming_message);
        CHECK_THAT(message.contains("jsonrpc") && message["jsonrpc"] == "2.0" && message.contains("method"));

        msg.id = message["id"];
        msg.client_id = message["params"]["client_id"];
        std::string mtd = message["method"].get<std::string>();

        auto mtd_id = string_to_command_id(mtd);
        if (!mtd_id.has_value())
        {
            ESP_LOGE("JSON-RPC", "Invalid method: %s", mtd.c_str());
            return ESP_FAIL;
        }
        msg.command = mtd_id.value();
        msg.params = message["params"];
        msg.params.erase("client_id");
        ESP_LOGI("JSON-RPC", "Received method: %s", magic_enum::enum_name(std::get<motor_command_id_t>(msg.command)).data());
        return ESP_OK;
    }
    catch (json::parse_error &e)
    {
        ESP_LOGE("JSON-RPC", "JSON parsing error: %s", e.what());
    }
    catch (json::type_error &e)
    {
        ESP_LOGE("JSON-RPC", "JSON type error: %s", e.what());
    }
    catch (json::exception &e)
    {
        ESP_LOGE("JSON-RPC", "JSON exception: %s", e.what());
    }
    catch (std::exception &e)
    {
        ESP_LOGE("JSON-RPC", "Exception: %s", e.what());
    }
    catch (...)
    {
        ESP_LOGE("JSON-RPC", "Unknown exception");
    }
    return ESP_FAIL;
}

void log_all_headers(httpd_req_t *req)
{

    ESP_LOGI(TAG, "Received headers:");

    char buf[100];
    size_t buf_len;

    const char *headers[] = {
        "Host",
        "Connection",
        "Upgrade",
        "Sec-WebSocket-Protocol",
        "Sec-WebSocket-Key",
        "Sec-WebSocket-Version",
        "Sec-WebSocket-Accept",
        "Sec-WebSocket-Extensions"};

    for (const char *header : headers)
    {
        buf_len = httpd_req_get_hdr_value_len(req, header) + 1;
        if (buf_len > 1)
        {
            httpd_req_get_hdr_value_str(req, header, buf, buf_len);
            ESP_LOGI(TAG, "%s: %s", header, buf);
        }
        else
        {
            ESP_LOGI(TAG, "%s header not found", header);
        }
    }
}

std::string payload_type_to_openapi(PayloadType type)
{
    switch (type)
    {
    case PayloadType::UINT8:
        return "type: integer\n        format: int32";
    case PayloadType::UINT16:
        return "type: integer\n        format: int32";
    case PayloadType::INT24:
        return "type: integer\n        format: int32";
    default:
        return ""; // VOID or unrecognized
    }
}

std::string generate_openapi_yaml()
{
    std::stringstream yaml;
    yaml << "openapi: 3.0.0\n";
    yaml << "info:\n";
    yaml << "  title: Robot Arm WebSocket API\n";
    yaml << "  version: 1.0.0\n";
    yaml << "paths:\n";
    yaml << "  /ws:\n";
    yaml << "    post:\n";
    yaml << "      summary: Execute a command on the WebSocket\n";
    yaml << "      requestBody:\n";
    yaml << "        required: true\n";
    yaml << "        content:\n";
    yaml << "          application/json:\n";
    yaml << "            schema:\n";
    yaml << "              oneOf:\n";

    // Add command schemas to oneOf
    for (const auto &[command, _] : g_command_payload_map)
    {
        std::string command_name = std::string(magic_enum::enum_name(command));
        yaml << "                - $ref: '#/components/schemas/" << command_name << "'\n";
    }

    yaml << "              discriminator:\n";
    yaml << "                propertyName: method\n";
    yaml << "      responses:\n";
    yaml << "        '200':\n";
    yaml << "          description: Command executed successfully\n";
    yaml << "        '400':\n";
    yaml << "          description: Bad request - Invalid command or parameters\n";

    // Ensure a single components.schemas section
    yaml << "components:\n";
    yaml << "  schemas:\n";

    // Add command schemas under components.schemas
    for (const auto &[command, payload_info] : g_command_payload_map)
    {
        std::string command_name = std::string(magic_enum::enum_name(command));
        yaml << "    " << command_name << ":\n";
        yaml << "      type: object\n";
        yaml << "      required:\n";
        yaml << "        - method\n";

        // Add parameters to 'required' if they exist
        size_t param_count = 0;
        for (const auto &type : payload_info.type_info)
        {
            if (type != PayloadType::VOID)
            {
                yaml << "        - param_" << param_count << "\n";
                param_count++;
            }
            else
            {
                break;
            }
        }

        yaml << "      properties:\n";
        yaml << "        method:\n";
        yaml << "          type: string\n";
        yaml << "          enum:\n";
        yaml << "            - " << command_name << "\n";

        // Add parameters with correct indentation
        param_count = 0;
        for (const auto &type : payload_info.type_info)
        {
            if (type != PayloadType::VOID)
            {
                yaml << "        param_" << param_count << ":\n";
                yaml << "          type: integer\n";
                yaml << "          format: int32\n";
                param_count++;
            }
            else
            {
                break;
            }
        }
    }

    return yaml.str();
}

std::optional<ws_command_id_t> string_to_command_id(const std::string &commandStr)
{

    if (auto system_command = magic_enum::enum_cast<system_command_id_t>(commandStr))
    {
        return *system_command;
    }
    if (auto motor_command = magic_enum::enum_cast<motor_command_id_t>(commandStr))
    {
        return *motor_command;
    }

    return std::nullopt;
}