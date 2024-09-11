#include <driver/twai.h>

class ICommandBuilder
{
private:
    
public:
    ICommandBuilder() {}
    virtual twai_message_t buildTwaiMessage() = 0;
    virtual ~ICommandBuilder() {}
};