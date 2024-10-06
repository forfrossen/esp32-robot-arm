#include "CommandStateMachine.hpp"

#define ENUM_TO_STRING_CASE(value) \
    case value:                    \
        return #value

const char *CommandStateMachine::stateToString(CommandStateMachine::CommandState state)
{

    switch (state)
    {
        ENUM_TO_STRING_CASE(CommandState::IDLE);
        ENUM_TO_STRING_CASE(CommandState::REQUESTED);
        ENUM_TO_STRING_CASE(CommandState::MOVING);
        ENUM_TO_STRING_CASE(CommandState::COMPLETED);
        ENUM_TO_STRING_CASE(CommandState::ERROR);
    default:
        return "UNKNOWN_VALUE";
    }
}

CommandStateMachine::CommandStateMachine() : state(CommandState::IDLE) {}

CommandStateMachine::CommandState CommandStateMachine::get_command_state() const
{
    return state;
}

void CommandStateMachine::set_command_state(CommandState newState)
{
    auto it = transitions.find(state);
    if (it == transitions.end())
    {
        ESP_LOGE(FUNCTION_NAME, "No transitions found for state %s", stateToString(state));
        return;
    }

    const std::vector<CommandState> &validTransitions = it->second;
    if (std::find(validTransitions.begin(), validTransitions.end(), newState) != validTransitions.end())
    {
        state = newState;
        ESP_LOGI(FUNCTION_NAME, "State changed to %s", stateToString(state));
    }
    else
    {
        ESP_LOGE(FUNCTION_NAME, "Invalid transition from %s to %s", stateToString(state), stateToString(newState));
    }
}
