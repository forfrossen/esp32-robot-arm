#include "StateMachine.hpp"
#include <iostream>
#include "utils.hpp"

#define ENUM_TO_STRING_CASE(value) \
  case value:                      \
    return #value

const char *StateMachine::stateToString(StateMachine::State state)
{

  switch (state)
  {
    ENUM_TO_STRING_CASE(State::IDLE);
    ENUM_TO_STRING_CASE(State::REQUESTED);
    ENUM_TO_STRING_CASE(State::MOVING);
    ENUM_TO_STRING_CASE(State::COMPLETED);
    ENUM_TO_STRING_CASE(State::ERROR);
  default:
    return "UNKNOWN_VALUE";
  }
}

StateMachine::StateMachine() : state(State::IDLE) {}

StateMachine::State StateMachine::getState() const
{
  return state;
}

void StateMachine::setState(State newState)
{
  auto it = transitions.find(state);
  if (it == transitions.end())
  {
    ESP_LOGE(FUNCTION_NAME, "No transitions found for state %s", stateToString(state));
    return;
  }

  const std::vector<State> &validTransitions = it->second;
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
