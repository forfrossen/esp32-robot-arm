idf_component_register(
  SRCS     
    "TWAIController.cpp"
REQUIRES driver
REQUIRES esp_event
INCLUDE_DIRS "." "../common")