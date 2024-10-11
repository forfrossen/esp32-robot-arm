idf_component_register(
  SRCS 
   "Controller.cpp"
   "Context.cpp"
   "ResponseHandler.cpp"   
REQUIRES driver
REQUIRES esp_event
REQUIRES esp_wifi
REQUIRES esp_http_server
INCLUDE_DIRS "." "../common" "../twai" "../commands" "../commands/SetParameters" "../magic_enum" "../magic_enum/include")