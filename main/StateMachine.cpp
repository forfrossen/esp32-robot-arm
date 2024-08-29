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
  auto transition = transitions.find({state, newState});
  if (transition != transitions.end())
  {
    transition->second(); // Execute the action
  }
  ESP_LOGI(FUNCTION_NAME, "State changed from %s to: %s", stateToString(state), stateToString(state));
  state = newState;
}

void StateMachine::addTransition(State from, State to, Action action)
{
  transitions[{from, to}] = action;
}