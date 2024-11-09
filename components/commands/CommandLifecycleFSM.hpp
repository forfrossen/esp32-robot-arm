#ifndef COMMAND_LIFECYCLE_FSM_H
#define COMMAND_LIFECYCLE_FSM_H

#include "TypeDefs.hpp"
#include "utils.hpp"
#include <esp_err.h>
#include <esp_log.h>
#include <functional>
#include <magic_enum.hpp>
#include <map>

class CommandLifecycleFSM
{

public:
    CommandLifecycleFSM() {};
    ~CommandLifecycleFSM() {};

    esp_err_t next_state()
    {
        auto it = transitions.find(state);
        CHECK_THAT(it != transitions.end());
        const NextStateTransitions &next_state_transitions = it->second;
        if (std::holds_alternative<CommandLifecycleState>(next_state_transitions))
        {
            state = std::get<CommandLifecycleState>(next_state_transitions);
        }
        else
        {
            ESP_LOGE(FUNCTION_NAME, "Invalid state transition to %s", stateToString(std::get<CommandLifecycleState>(next_state_transitions)));
            return ESP_FAIL;
        }
        return ESP_OK;
    }

    esp_err_t transition_to_error()
    {
        state = CommandLifecycleState::ERROR;
        ESP_LOGE(FUNCTION_NAME, "Transitioned to ERROR state");
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

    std::map<CommandLifecycleState, NextStateTransitions> transitions = {
        {CommandLifecycleState::CREATED, {CommandLifecycleState::SENT}},
        {CommandLifecycleState::SENT, {CommandLifecycleState::EXECUTING}},
        {CommandLifecycleState::EXECUTING, {CommandLifecycleState::PROCESSED}},
        {CommandLifecycleState::TIMEOUT, {CommandLifecycleState::EXECUTING}},
        {CommandLifecycleState::UNKNOWN, {CommandLifecycleState::CREATED}}};
};

#endif // COMMAND_LIFECYCLE_FSM_H