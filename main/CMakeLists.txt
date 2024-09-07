idf_component_register(
  SRCS 
    "StateMachine.cpp"
    "CanBus.cpp"
    "CANServo.cpp"
    "Wifi.cpp"
    "main.cpp"
REQUIRES driver
REQUIRES esp_event
REQUIRES esp_wifi
REQUIRES nvs_flash
INCLUDE_DIRS ".")