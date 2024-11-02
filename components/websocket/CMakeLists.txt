idf_component_register(
  SRCS
    "WebSocketServer.cpp"
    "EventManager.cpp"
    "RequestHandler.cpp"
    "ResponseSender.cpp"
    "Utilities.cpp"
    "WebSocket.cpp"

REQUIRES esp_event
REQUIRES esp_wifi
REQUIRES esp_http_server
#REQUIRES nlohmann-json
REQUIRES johboh__nlohmann-json

INCLUDE_DIRS 
    "." 
    "../magic_enum"
    "../common"
    "./commands"
    
)

target_compile_definitions(${COMPONENT_LIB} PRIVATE JSON_BuildTests=OFF JSON_ImplicitConversions=OFF)
