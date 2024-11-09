idf_component_register(
  SRCS 
    "DataExtractor.cpp"

REQUIRES driver
REQUIRES esp_event
REQUIRES esp_wifi
REQUIRES esp_http_server
REQUIRES common
REQUIRES motor
REQUIRES twai
REQUIRES websocket 
REQUIRES magic_enum
REQUIRES commands

INCLUDE_DIRS .
)
