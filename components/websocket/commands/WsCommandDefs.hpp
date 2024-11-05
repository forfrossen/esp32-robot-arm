#pragma once

#include "MksEnums.hpp"
#include "esp_event.h"
#include <map>

#include "../../managed_components/johboh__nlohmann-json/single_include/nlohmann/json.hpp"
#include "TypeDefs.hpp"
#include <any>
#include <array>
#include <esp_event.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <map>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

enum class ws_command_id
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
typedef struct
{
    std::variant<ws_command_id, motor_command_id_t> command;
    nlohmann::json params;
    int id;
    std::string client_id;
} ws_message_t;

using ws_command_config_map_t = std::map<ws_command_id, rpc_event_config_t>;

typedef struct
{
    class IWsCommand *command;
} rpc_event_data;

typedef struct
{
    std::string client_id;
    int message_count;
} ws_client_info;