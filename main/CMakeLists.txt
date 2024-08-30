idf_component_register(
  SRCS 
    "StateMachine.cpp"
    "CanBus.cpp"
    "CANServo.cpp"
    "main.cpp"
REQUIRES driver
INCLUDE_DIRS ".")