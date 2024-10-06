#include "Events.hpp"
extern "C"
{
    // Define the event base
    ESP_EVENT_DEFINE_BASE(SYSTEM_EVENT);
    ESP_EVENT_DEFINE_BASE(MOTOR_EVENT);
}