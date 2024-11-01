#ifndef UTILITIES_H
#define UTILITIES_H

#include "../magic_enum/include/magic_enum/magic_enum.hpp"
#include "WsCommandDefs.hpp"
#include "cJSON.h"
#include "esp_err.h"
#include "esp_log.h"
#include "utils.hpp"
#include <string>
#include <utility>

esp_err_t parse_json_msg(const std::string &message, ws_message_t &msg);
esp_err_t payload_is_valid(ws_payload_t &payload);
esp_err_t get_run_mode(ws_payload_t &payload, RunMode &run_mode);
#endif // UTILITIES_H
