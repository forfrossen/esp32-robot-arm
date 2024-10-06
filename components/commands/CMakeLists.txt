idf_component_register(
  SRCS 
    "CommandStateMachine.cpp"
REQUIRES driver
REQUIRES esp_event

INCLUDE_DIRS "." "./SetParameters")