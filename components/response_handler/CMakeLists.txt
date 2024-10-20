idf_component_register(
  SRCS 
    "DataExtractor.cpp"

REQUIRES driver
REQUIRES esp_event
REQUIRES esp_wifi
REQUIRES esp_http_server
INCLUDE_DIRS "." "../common" "../twai" "../commands" "../magic_enum" "../magic_enum/include")
