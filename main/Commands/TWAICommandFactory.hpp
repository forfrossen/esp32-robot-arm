#include "Query/QueryMotorPositionCommandBuilder.hpp"
#include "SetTargetPositionCommandBuilder.hpp"
#include "freertos/queue.h" // Include the header file that defines QueueHandle_t
#include <driver/twai.h>

class TWAICommandFactory
{
private:
    uint32_t id;
    QueueHandle_t outQ;

public:
    // Konstruktor setzt den Identifier
    TWAICommandFactory(uint32_t id, QueueHandle_t outQ) : id(id), outQ(outQ)
    {
    }

    SetTargetPositionCommandBuilder createSetTargetPositionCommand()
    {
        return SetTargetPositionCommandBuilder(id, outQ);
    }

    QueryMotorPositionCommandBuilder createQueryMotorPositionCommand()
    {
        return QueryMotorPositionCommandBuilder(id, outQ);
    }
};