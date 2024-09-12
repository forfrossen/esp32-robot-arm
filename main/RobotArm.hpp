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
    RobotArm()
    {

        twai_controller = new TWAIController();
        command_mapper = new CommandMapper();
        vTaskDelay(pdMS_TO_TICKS(1000));

        // Servos[0x01] = new MotorController(0x01, twai_controller, command_mapper);
        servos[2] = new MotorController(0x02, twai_controller, command_mapper);
        // Servos[0x03] = new MotorController(0x03, twai_controller, command_mapper);
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