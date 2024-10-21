#ifndef COMMAND_LIFECYCLE_FSM_H
#define COMMAND_LIFECYCLE_FSM_H

#include "../common/utils.hpp"
#include "../magic_enum/include/magic_enum/magic_enum.hpp"
#include "TypeDefs.hpp"
#include "esp_err.h"
#include "esp_log.h"
#include <functional>
#include <map>

class CommandLifecycleFSM
{

public:
    CommandLifecycleFSM() {};
    ~CommandLifecycleFSM() {};

    esp_err_t next_state()
    {
        auto next_state = transitions.find(state);
        CHECK_THAT(next_state != transitions.end());
        state = next_state->second;
        return ESP_OK;
    }

    esp_err_t transition_to_error(CommandLifecycleState new_state)
    {
        CHECK_THAT(state != new_state);
        auto next_state = transitions.find(state);
        CHECK_THAT(next_state != transitions.end());

        if (new_state == CommandLifecycleState::UNKNOWN || new_state == CommandLifecycleState::ERROR || new_state == CommandLifecycleState::TIMEOUT)
        {
            ESP_LOGE(FUNCTION_NAME, "Invalid state transition from: %s to: %s", stateToString(state), stateToString(new_state));
            state = new_state;
        }

        ESP_LOGI(FUNCTION_NAME, "Transitioning from: %s to: %s", stateToString(state), stateToString(new_state));
        state = new_state;
        return ESP_OK;
    }

private:
    const char *stateToString(CommandLifecycleState CommandLifecycleState)
    {
        return magic_enum::enum_name(CommandLifecycleState).data();
        // switch (CommandLifecycleState)
        // {
        // case CommandLifecycleState::CREATED:
        //     return "CREATED";
        // case CommandLifecycleState::SENT:
        //     return "SENT";
        // case CommandLifecycleState::EXECUTING:
        //     return "EXECUTING";
        // case CommandLifecycleState::PROCESSED:
        //     return "PROCESSED";
        // case CommandLifecycleState::ERROR:
        //     return "ERROR";
        // case CommandLifecycleState::TIMEOUT:
        //     return "TIMEOUT";
        // case CommandLifecycleState::UNKNOWN:
        //     return "UNKNOWN";
        // default:
        //     return "UNKNOWN";
        // }
    };

    CommandLifecycleState state = CommandLifecycleState::CREATED;

    std::map<CommandLifecycleState, CommandLifecycleState> transitions = {
        {CommandLifecycleState::CREATED, CommandLifecycleState::SENT},
        {CommandLifecycleState::SENT, CommandLifecycleState::EXECUTING},
        {CommandLifecycleState::EXECUTING, CommandLifecycleState::PROCESSED},
        {CommandLifecycleState::TIMEOUT, CommandLifecycleState::EXECUTING},
        {CommandLifecycleState::UNKNOWN, CommandLifecycleState::CREATED}};
};

#endif // COMMAND_LIFECYCLE_FSM_H