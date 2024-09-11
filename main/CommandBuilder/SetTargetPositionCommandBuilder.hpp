#include "ICommandBuilder.hpp"
#include <driver/twai.h>

class SetTargetPositionCommandBuilder : public ICommandBuilder
{
private:
    int position;
    int speed;
    int acceleration;
    bool absolute;

    twai_message_t data;

public:
    SetTargetPositionCommandBuilder &setPosition(int pos)
    {
        position = pos;
        return *this;
    }

    SetTargetPositionCommandBuilder &setSpeed(int spd)
    {
        speed = spd;
        return *this;
    }

    SetTargetPositionCommandBuilder &setAcceleration(int accel)
    {
        acceleration = accel;
        return *this;
    }

    SetTargetPositionCommandBuilder &setAbsolute(bool abs)
    {
        absolute = abs;
        return *this;
    }

    twai_message_t buildTwaiMessage(unit8_t data) override
    {
        data.resize(8); // Stellen Sie sicher, dass der Datenvektor die richtige Größe hat

        data[0] = absolute ? 0xF5 : 0xF4;
        data[1] = (speed >> 8) & 0x7F;
        data[2] = speed & 0xFF;
        data[3] = acceleration;
        data[4] = (position >> 16) & 0xFF;
        data[5] = (position >> 8) & 0xFF;
        data[6] = position & 0xFF;
        data[7] = calculateCRC(data); // Implementieren Sie Ihre CRC-Berechnungsfunktion

        twai_message_t message;
        message.identifier = 0x123; // Setzen Sie hier den gewünschten Identifier
        message.data_length_code = data.size();
        std::copy(data.begin(), data.end(), message.data);

        return message;
    }
};