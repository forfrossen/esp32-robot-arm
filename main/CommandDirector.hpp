#include <driver/twai.h>
#include <map>
#include <memory>
#include <string>

#include "CommandBuilder/ICommandBuilder.hpp"
#include "esp_err.h"
#include "esp_log.h"
#include "utils.hpp"

class TWAICommandDirector
{
private:
    static TWAICommandDirector *instance;
    std::map<std::string, std::unique_ptr<ICommandBuilder>> builders;

    // Private Konstruktor verhindert das Erzeugen von mehreren Instanzen
    TWAICommandDirector()
    {
        // Registriere alle verfügbaren Builder hier
    }

public:
    // Verhindere die Kopie oder Zuweisung
    TWAICommandDirector(const TWAICommandDirector &) = delete;
    TWAICommandDirector &operator=(const TWAICommandDirector &) = delete;

    static TWAICommandDirector *getInstance()
    {
        if (instance == nullptr)
        {
            instance = new TWAICommandDirector();
        }
        return instance;
    }

    twai_message_t buildCommand(const std::string &commandType, uint32_t identifier, const CommandParameters &params)
    {
        auto it = builders.find(commandType);
        if (it != builders.end())
        {
            it->second->setIdentifier(identifier);
            it->second->setParameters(params);
            return it->second->buildTwaiMessage();
        }
        ESP_LOGE(FUNCTION_NAME; "Unbekannter Kommandotyp: %s", commandType);
    }
};

// Initialisiere den statischen Pointer als nullptr am Anfang
TWAICommandDirector *TWAICommandDirector::instance = nullptr;