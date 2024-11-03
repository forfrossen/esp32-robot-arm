#ifndef EVENTS_HPP
#define EVENTS_HPP

extern "C"
{
#include "esp_event.h"

    // Declare the event base for motor controller events
    ESP_EVENT_DECLARE_BASE(SYSTEM_EVENTS);
    ESP_EVENT_DECLARE_BASE(RPC_EVENTS);
    ESP_EVENT_DECLARE_BASE(MOTOR_EVENTS);
}

#endif // EVENTS_HPP