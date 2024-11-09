idf_component_register(
  SRCS     
    "TWAIController.cpp"
    
REQUIRES driver
REQUIRES esp_event
REQUIRES common
REQUIRES motor
REQUIRES twai
REQUIRES wifi
REQUIRES websocket 
REQUIRES magic_enum
REQUIRES response_handler

INCLUDE_DIRS .
)