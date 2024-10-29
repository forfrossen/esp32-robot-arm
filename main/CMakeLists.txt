idf_component_register(
  SRCS 
    "main.cpp"
REQUIRES driver
REQUIRES esp_event
REQUIRES esp_wifi
REQUIRES esp_http_server
REQUIRES nvs_flash
REQUIRES json

INCLUDE_DIRS 
    "." 
    "../components/commands" 
    "../components/common"
    "../components/motor"
    "../components/twai"
    "../components/wifi"
    "../components/websocket"
)