#pragma once

#ifndef WS_COMMAND_DEFS_H
#define WS_COMMAND_DEFS_H

#include "MksEnums.hpp"
#include <esp_event.h>
#include <map>

#include "TypeDefs.hpp"
#include <any>
#include <array>
#include <esp_event.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

enum class system_command_id_t
{
    START_MOTORS,
    STOP_MOTORS,
    SET_RUNLEVEL,
    SET_TARGET_POSITION,
    MOTOR_CONTROL_COMMAND,
    UNKNOWN
};

typedef struct
{
    esp_event_loop_handle_t loop;
    esp_event_base_t event_base;
    int32_t event_id;
} rpc_event_config_t;

typedef struct
{
    uint8_t motor_id;
    int32_t position;
    int32_t speed;
    int32_t acceleration;
} ws_set_target_position_payload_t;

// using ws_command_t = std::string;
// using ws_payload_t = std::variant<int, std::string, RunLevel>;
using ws_command_id_t = std::variant<system_command_id_t, motor_command_id_t>;
typedef struct
{
    ws_command_id_t command;
    nlohmann::json params;
    int id;
    std::string client_id;

} ws_message_t;

typedef struct
{
    class IWsCommand *command;
} rpc_event_data;

typedef struct
{
    std::string client_id;
    int message_count;
} ws_client_info;

using ws_command_config_map_t = std::map<ws_command_id_t, rpc_event_config_t>;

#endif // WS_COMMAND_DEFS_H