idf_component_register(
  SRCS 
   "Controller.cpp"
   "Context.cpp"
   "Builder.cpp"
   "ResponseHandler.cpp"   
   "Properties.cpp"
   "ControllerTasks.cpp"

REQUIRES driver
REQUIRES esp_event
REQUIRES esp_wifi
REQUIRES esp_http_server
REQUIRES common
REQUIRES commands
REQUIRES twai
REQUIRES wifi
REQUIRES websocket 
REQUIRES magic_enum
REQUIRES response_handler

INCLUDE_DIRS "include"
)

