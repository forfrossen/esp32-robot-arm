#ifndef COMMANDMAPPER_H
#define COMMANDMAPPER_H
#include "esp_err.h"
#include "esp_log.h"
#include "utils.hpp"
#include <cstdint>
#include <cstring>
#include <map>
#include <string>
#include <unordered_map>

class CommandMapper
{

private:
    static const std::unordered_map<uint8_t, const char *> command_map;

public:
    CommandMapper();

    void get_command_name_from_code(uint8_t code, char *command_name) const;
};
#endif