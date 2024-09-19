#ifndef COMMAND_LIFECYCLE_FSM_H
#define COMMAND_LIFECYCLE_FSM_H

#include "esp_err.h"
#include "esp_log.h"
#include "utils.hpp"
#include <functional>
#include <map>

class CommandLifecycleFSM
{
    enum class State
    {
        CREATED,
        SENT,
        RECEIVED,
        PROCESSED,
        ERROR,
        TIMEOUT,
        UNKNOWN
    };

public:
    CommandLifecycleFSM() {};
    ~CommandLifecycleFSM() {};

private:
    const char *stateToString(State state)
    {
        switch (state)
        {
        case State::CREATED:
            return "CREATED";
        case State::SENT:
            return "SENT";
        case State::RECEIVED:
            return "RECEIVED";
        case State::PROCESSED:
            return "PROCESSED";
        case State::ERROR:
            return "ERROR";
        case State::TIMEOUT:
            return "TIMEOUT";
        case State::UNKNOWN:
            return "UNKNOWN";
        default:
            return "UNKNOWN";
        }
    };

    State state;

    std::map<State, std::vector<State>> transitions = {
        {State::CREATED, {State::SENT, State::ERROR}},
        {State::SENT, {State::RECEIVED, State::ERROR, State::TIMEOUT}},
        {State::RECEIVED, {State::PROCESSED, State::ERROR}},
        {State::TIMEOUT, {State::PROCESSED, State::ERROR}},
        {State::UNKNOWN, {State::CREATED, State::ERROR}}};
};

#endif // COMMAND_LIFECYCLE_FSM_H