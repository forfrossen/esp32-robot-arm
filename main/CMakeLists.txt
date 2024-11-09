idf_component_register(
  SRCS 
    "main.cpp"
    
REQUIRES driver
REQUIRES esp_event
REQUIRES esp_wifi
REQUIRES esp_http_server
REQUIRES nvs_flash
REQUIRES common
REQUIRES motor
REQUIRES twai
REQUIRES wifi
REQUIRES websocket 
REQUIRES johboh__nlohmann-json

INCLUDE_DIRS .

)