#ifndef STATEMACHINE_HPP
#define STATEMACHINE_HPP

#include "esp_err.h"
#include "esp_log.h"
#include "utils.hpp"
#include <functional>
#include <map>

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

    StateMachine();

    State get_state() const;
    void set_state(State newState);
    void addTransition(State from, State to[]);

private:
    const char *stateToString(StateMachine::State state);

    State state;

    std::map<State, std::vector<State>> transitions = {
        {State::IDLE, {State::REQUESTED, State::ERROR}},
        {State::REQUESTED, {State::REQUESTED, State::MOVING, State::ERROR}},
        {State::MOVING, {State::COMPLETED, State::REQUESTED, State::ERROR}},
        {State::COMPLETED, {State::IDLE, State::REQUESTED}},
        {State::ERROR, {State::IDLE, State::REQUESTED}}};
};

#endif // STATEMACHINE_HPP