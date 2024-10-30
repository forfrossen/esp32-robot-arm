#pragma once

#include "ICommand.h"

class SetRunmodeCommand : public ICommand
{
public:
    explicit SetRunmodeCommand(int run_mode) : run_mode(run_mode) {}
    int get_run_mode() const { return run_mode; }

private:
    int run_mode;
};