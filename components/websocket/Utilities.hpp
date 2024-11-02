#ifndef UTILITIES_H
#define UTILITIES_H
#define JSON_SKIP_LIBRARY_VERSION_CHECK 1
#define JSON_DIAGNOSTICS 0 // Deaktiviert zusätzliche Debug-Informationen
#define JSON_NOEXCEPTION 1 // Deaktiviert Ausnahmebehandlung

#include "../../managed_components/johboh__nlohmann-json/single_include/nlohmann/json.hpp"
#include "../magic_enum/include/magic_enum/magic_enum.hpp"
#include "WsCommandDefs.hpp"
#include "esp_err.h"
#include "esp_log.h"
#include "utils.hpp"

#include <string>
#include <utility>

// #include "../managed_components/johboh__nlohmann-json/include/nlohmann/detail/conversions/from_json.hpp"
// #include <nlohmann/detail/conversions/to_json.hpp>
// #include <nlohmann/detail/macro_scope.hpp>
// #include <nlohmann/detail/meta/type_traits.hpp>
// #include <nlohmann/json_fwd.hpp>
// #include "../managed_components/johboh__nlohmann-json/include/nlohmann/detail/conversions/from_json.hpp"
// #include "../managed_components/johboh__nlohmann-json/include/nlohmann/detail/conversions/to_json.hpp"
// #include "../managed_components/johboh__nlohmann-json/include/nlohmann/detail/macro_scope.hpp"
// #include "../managed_components/johboh__nlohmann-json/include/nlohmann/detail/meta/type_traits.hpp"
// #include "../managed_components/johboh__nlohmann-json/include/nlohmann/json_fwd.hpp"

// #include <nlohmann/detail/conversions/from_json.hpp>
// #include <nlohmann/detail/conversions/to_json.hpp>
// #include <nlohmann/detail/macro_scope.hpp>
// #include <nlohmann/detail/meta/type_traits.hpp>

// #include "johboh__nlohmann-json/include/nlohmann/detail/conversions/from_json.hpp"
// #include "nlohmann/json_fwd.hpp"

esp_err_t parse_json_msg(const std::string &message, ws_message_t &msg);
esp_err_t payload_is_valid(ws_payload_t &payload);
esp_err_t get_run_mode(ws_payload_t &payload, RunMode &run_mode);
#endif // UTILITIES_H
