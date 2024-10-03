#ifndef EVENTS_HPP
#define EVENTS_HPP

extern "C"
{
#include "esp_event.h"

    // Declare the event base for motor controller events
    ESP_EVENT_DECLARE_BASE(MOTOR_EVENT);
    ESP_EVENT_DECLARE_BASE(SYSTEM_EVENT);
}

#endif // EVENTS_HPP