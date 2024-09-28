idf_component_register(
  SRCS 
    "TWAIController.cpp"
    "CommandStateMachine.cpp"
    "MotorController.cpp"
    "MotorContext.cpp"
    "MotorResponseHandler.cpp"
    "Wifi.cpp"
    "main.cpp"
REQUIRES driver
REQUIRES esp_event
REQUIRES esp_wifi
REQUIRES nvs_flash
REQUIRES esp_http_server
INCLUDE_DIRS "." "Commands" "Commands/SetParameters")