#ifndef COMMAND_LIFECYCLE_FSM_H
#define COMMAND_LIFECYCLE_FSM_H

#include "../common/utils.hpp"
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

private:
    const char *stateToString(CommandLifecycleState CommandLifecycleState)
    {
        switch (CommandLifecycleState)
        {
        case CommandLifecycleState::CREATED:
            return "CREATED";
        case CommandLifecycleState::SENT:
            return "SENT";
        case CommandLifecycleState::EXECUTING:
            return "EXECUTING";
        case CommandLifecycleState::PROCESSED:
            return "PROCESSED";
        case CommandLifecycleState::ERROR:
            return "ERROR";
        case CommandLifecycleState::TIMEOUT:
            return "TIMEOUT";
        case CommandLifecycleState::UNKNOWN:
            return "UNKNOWN";
        default:
            return "UNKNOWN";
        }
    };

    CommandLifecycleState state;

    std::map<CommandLifecycleState, std::vector<CommandLifecycleState>> transitions = {
        {CommandLifecycleState::CREATED, {CommandLifecycleState::SENT, CommandLifecycleState::ERROR}},
        {CommandLifecycleState::SENT, {CommandLifecycleState::EXECUTING, CommandLifecycleState::ERROR, CommandLifecycleState::TIMEOUT}},
        {CommandLifecycleState::EXECUTING, {CommandLifecycleState::PROCESSED, CommandLifecycleState::ERROR}},
        {CommandLifecycleState::TIMEOUT, {CommandLifecycleState::EXECUTING, CommandLifecycleState::PROCESSED, CommandLifecycleState::ERROR}},
        {CommandLifecycleState::UNKNOWN, {CommandLifecycleState::CREATED, CommandLifecycleState::ERROR}}};
};

#endif // COMMAND_LIFECYCLE_FSM_H