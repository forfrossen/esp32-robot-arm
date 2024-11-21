idf_component_register(
  SRCS 
    "Wifi.cpp"

REQUIRES esp_wifi
REQUIRES nvs_flash
REQUIRES common

INCLUDE_DIRS "."

)