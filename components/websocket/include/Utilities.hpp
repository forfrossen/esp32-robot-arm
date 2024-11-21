#pragma once
#ifndef UTILITIES_H
#define UTILITIES_H
#define JSON_SKIP_LIBRARY_VERSION_CHECK 1
#define JSON_DIAGNOSTICS 0
#define JSON_NOEXCEPTION 1

#include "CommandPayloadTypeDefs.hpp"
#include "TypeDefs.hpp"
#include "WsCommandDefs.hpp"
#include "utils.hpp"
#include <esp_check.h>
#include <esp_err.h>
#include <esp_http_server.h>
#include <esp_log.h>
#include <esp_random.h>
#include <esp_system.h>
#include <iomanip>
#include <magic_enum.hpp>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <utility>

using json = nlohmann::json;

esp_err_t receive_frame(httpd_req_t *req, httpd_ws_frame_t &ws_pkt, uint8_t *&buf);
esp_err_t parse_json_msg(const std::string &message, ws_message_t &msg);
esp_err_t parse_json_rpc(const std::string &incoming_message, ws_message_t &msg);
esp_err_t payload_is_valid(nlohmann::json &payload);
esp_err_t get_run_level_from_json(nlohmann::json &payload, RunLevel &run_level);
esp_err_t get_cookie_value(const char *cookie_header, const std::string &key, std::string &cookie_value);

std::string payload_type_to_openapi(PayloadType type);
std::string generate_openapi_yaml();

void log_all_headers(httpd_req_t *req);
std::string generate_uuid();
std::optional<ws_command_id_t> string_to_command_id(const std::string &command_str);

#endif // UTILITIES_H
