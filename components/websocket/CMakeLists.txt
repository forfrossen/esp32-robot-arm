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
REQUIRES json

INCLUDE_DIRS 
    "." 
    "../magic_enum"
    "../common"
    "./commands"
)