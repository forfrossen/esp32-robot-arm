idf_component_register(
  SRCS 
    "CommandStateMachine.cpp"
REQUIRES driver
REQUIRES esp_event
REQUIRES magic_enum
INCLUDE_DIRS "." "../common")