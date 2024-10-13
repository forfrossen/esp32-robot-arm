idf_component_register(
  SRCS 
    "main.cpp"
    "../components/common/Events.cpp"
REQUIRES driver
REQUIRES esp_event
REQUIRES esp_wifi
REQUIRES esp_http_server
REQUIRES nvs_flash
INCLUDE_DIRS 
    "." 
    "../components/commands" 
    "../components/common"
    "../components/motor"
    "../components/twai"
    "../components/websocket"
    "../components/wifi"
)