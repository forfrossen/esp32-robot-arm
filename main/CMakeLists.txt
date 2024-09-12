idf_component_register(
  SRCS 
    "TWAIController.cpp"
    "MotorController.cpp"
    "StateMachine.cpp" 
    "Wifi.cpp"
    "main.cpp"
REQUIRES driver
REQUIRES esp_event
REQUIRES esp_wifi
REQUIRES nvs_flash
INCLUDE_DIRS "." "Commands" "Commands/Query" "Commands/SetParameters")