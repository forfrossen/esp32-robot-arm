idf_component_register(
  SRCS 
    "WebSocketServer.cpp"

REQUIRES esp_event
REQUIRES esp_wifi
REQUIRES esp_http_server
REQUIRES json

INCLUDE_DIRS 
    "." 
    "../magic_enum"
    "../common"
)