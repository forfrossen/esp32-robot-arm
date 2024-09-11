#include "CommandMapper.hpp"
#include "MotorController.hpp"
#include "TWAIController.hpp"
// #include "Kinematics.hpp"

class RobotArm
{
private:
    std::map<uint8_t, MotorController *> servos;
    TWAIController *twai_controller;
    CommandMapper *command_mapper;

    // Kinematics kinematics;          // Klasse für kinematische Berechnungen
public:
    RobotArm(TWAIController *twai_controller, CommandMapper *command_mapper)
    {
        this->twai_controller = twai_controller;
        this->command_mapper = command_mapper;

        servos[2] = new MotorController(0x02, twai_controller, command_mapper);
    }

    ~RobotArm()
    {
        // Speicherfreigabe für die Servo-Objekte
        for (auto &servo : servos)
        {
            delete servo.second;
        }
    }
};