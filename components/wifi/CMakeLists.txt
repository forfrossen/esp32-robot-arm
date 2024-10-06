idf_component_register(
  SRCS 
    "Wifi.cpp"

REQUIRES esp_wifi
REQUIRES nvs_flash

INCLUDE_DIRS "." "../common" )