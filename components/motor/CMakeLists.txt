idf_component_register(
  SRCS 
   "Controller.cpp"
   "Context.cpp"
   "ResponseHandler.cpp"   
   "Properties.cpp"
   "ControllerTasks.cpp"
REQUIRES driver
REQUIRES esp_event
REQUIRES esp_wifi
REQUIRES esp_http_server
INCLUDE_DIRS "." "../common" "../twai" "../commands" "../magic_enum" "../magic_enum/include" "../response_handler")

target_compile_definitions(${COMPONENT_LIB} PRIVATE TAG="Motor")
