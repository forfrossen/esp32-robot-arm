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

enum class ws_command_id
{
    START_MOTORS,
    STOP_MOTORS,
    SET_RUNMODE,
    SET_TARGET_POSITION,
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
    ws_command_id command;
    nlohmann::json params;
    int id;
} ws_message_t;

using ws_command_config_map_t = std::map<ws_command_id, rpc_event_config_t>;

typedef struct
{
    class IWsCommand *command;
} rpc_event_data;

typedef struct
{
    std::string client_id;
} ws_session_t;