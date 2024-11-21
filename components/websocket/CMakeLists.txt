idf_component_register(
  SRCS
    "WebSocketServer.cpp"
    "EventManager.cpp"
    "RequestHandler.cpp"
    "ResponseSender.cpp"
    "Utilities.cpp"
    "WebSocket.cpp"
    "ClientManager.cpp"

REQUIRES esp_event
REQUIRES nvs_flash
REQUIRES esp_wifi
REQUIRES esp_http_server
REQUIRES johboh__nlohmann-json
REQUIRES common
REQUIRES magic_enum

INCLUDE_DIRS 
    "include"
    "commands"    
)

target_compile_definitions(${COMPONENT_LIB} PRIVATE JSON_BuildTests=OFF JSON_ImplicitConversions=OFF)
