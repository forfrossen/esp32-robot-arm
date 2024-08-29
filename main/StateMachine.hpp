#ifndef STATEMACHINE_HPP
#define STATEMACHINE_HPP

#include <functional>
#include <map>
#include "esp_err.h"
#include "esp_log.h"
#include "utils.hpp"

class StateMachine
{
public:
  enum class State
  {
    IDLE,
    REQUESTED,
    MOVING,
    COMPLETED,
    ERROR
  };

  using Action = std::function<void()>;

  StateMachine();

  State getState() const;
  void setState(State newState);
  void addTransition(State from, State to, Action action);

private:
  const char *stateToString(StateMachine::State state);
  struct Transition
  {
    State from;
    State to;
    Action action;
  };

  State state;
  std::map<std::pair<State, State>, Action> transitions;
};

#endif // STATEMACHINE_HPP