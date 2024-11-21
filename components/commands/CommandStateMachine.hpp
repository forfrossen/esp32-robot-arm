#ifndef STATEMACHINE_HPP
#define STATEMACHINE_HPP

#include "utils.hpp"
#include <esp_err.h>
#include <esp_log.h>
#include <functional>
#include <iostream>
#include <map>

class CommandStateMachine
{
public:
    enum class CommandState
    {
        IDLE,
        REQUESTED,
        MOVING,
        COMPLETED,
        ERROR
    };

    CommandStateMachine();

    CommandState get_command_state() const;
    void set_command_state(CommandState newState);
    void addTransition(CommandState from, CommandState to[]);

private:
    const char *stateToString(CommandStateMachine::CommandState state);

    CommandState state;

    std::map<CommandState, std::vector<CommandState>> transitions = {
        {CommandState::IDLE, {CommandState::REQUESTED, CommandState::ERROR}},
        {CommandState::REQUESTED, {CommandState::REQUESTED, CommandState::MOVING, CommandState::ERROR}},
        {CommandState::MOVING, {CommandState::COMPLETED, CommandState::REQUESTED, CommandState::ERROR}},
        {CommandState::COMPLETED, {CommandState::IDLE, CommandState::REQUESTED}},
        {CommandState::ERROR, {CommandState::IDLE, CommandState::REQUESTED}}};
};

#endif // STATEMACHINE_HPP