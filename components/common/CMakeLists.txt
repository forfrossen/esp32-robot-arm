idf_component_register(
  SRCS 
    "Events.cpp"
REQUIRES driver
REQUIRES esp_event

REQUIRES magic_enum

INCLUDE_DIRS .
)