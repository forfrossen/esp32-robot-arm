#ifndef UTILITIES_H
#define UTILITIES_H

#include "esp_err.h"
#include <string>
#include <utility>

typedef std::pair<std::string, std::string> ws_message_t;
typedef std::string ws_payload_t;

// Function to split WebSocket message into command and payload
esp_err_t split_ws_msg(const std::string &message, ws_message_t &msg);

#endif // UTILITIES_H

