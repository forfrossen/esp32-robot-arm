#ifndef MKS_ENUMS_HPP
#define MKS_ENUMS_HPP

#include <cstdint>

enum class CommandIds : uint8_t
{
    MOTOR_CALIBRATION = 0x80,
    SET_WORK_MODE = 0x82,
    SET_WORKING_CURRENT = 0x83,
    SET_HOLDING_CURRENT = 0x9B,
    SET_SUBDIVISIONS = 0x84,
    SET_EN_PIN_CONFIG = 0x85,
    SET_MOTOR_ROTATION_DIRECTION = 0x86,
    SET_AUTO_TURN_OFF_SCREEN = 0x87,
    SET_MOTOR_SHAFT_LOCKED_ROTOR_PROTECTION = 0x88,
    SET_SUBDIVISION_INTERPOLATION = 0x89,
    SET_CAN_BITRATE = 0x8A,
    SET_CAN_ID = 0x8B,
    SET_KEY_LOCK_ENABLE = 0x8F,
    SET_GROUP_ID = 0x8D,
    SET_HOME = 0x90,
    GO_HOME = 0x91,
    SET_CURRENT_AXIS_TO_ZERO = 0x92,
    SET_LIMIT_PORT_REMAP = 0x9E,
    SET_MODE0 = 0x9A,
    RESTORE_DEFAULT_PARAMETERS = 0x3F,
    READ_ENCODER_VALUE_CARRY = 0x30,
    READ_ENCODED_VALUE_ADDITION = 0x31,
    READ_MOTOR_SPEED = 0x32,
    READ_NUM_PULSES_RECEIVED = 0x33,
    READ_IO_PORT_STATUS = 0x34,
    READ_MOTOR_SHAFT_ANGLE_ERROR = 0x39,
    READ_EN_PINS_STATUS = 0x3A,
    READ_GO_BACK_TO_ZERO_STATUS_WHEN_POWER_ON = 0x3B,
    RELEASE_MOTOR_SHAFT_LOCKED_PROTECTION_STATE = 0x3D,
    READ_MOTOR_SHAFT_PROTECTION_STATE = 0x3E,
    QUERY_MOTOR_STATUS = 0xF1,
    ENABLE_MOTOR = 0xF3,
    EMERGENCY_STOP = 0xF7,
    RUN_MOTOR_SPEED_MODE = 0xF6,
    SAVE_CLEAN_IN_SPEED_MODE = 0xFF,
    RUN_MOTOR_RELATIVE_MOTION_BY_PULSES = 0xFD,
    RUN_MOTOR_ABSOLUTE_MOTION_BY_PULSES = 0xFE,
    RUN_MOTOR_RELATIVE_MOTION_BY_AXIS = 0xF4,
    RUN_MOTOR_ABSOLUTE_MOTION_BY_AXIS = 0xF5
};

enum class Direction : uint8_t
{
    CW = 0, // Clockwise
    CCW = 1 // Counter-Clockwise
};

enum class Enable : uint8_t
{
    Disable = 0,
    Enable = 1
};

enum class EnableStatus : uint8_t
{
    Disabled = 0,
    Enabled = 1
};

enum class SuccessStatus : uint8_t
{
    Fail = 0,
    Success = 1
};

enum class GoBackToZeroStatus : uint8_t
{
    GoingToZero = 0,
    GoBackToZeroSuccess = 1,
    GoBackToZeroFail = 2
};

enum class StatusCommand8 : uint8_t
{
    ReleaseFails = 0,
    ReleaseSuccess = 1
};

enum class StatusCommand9 : uint8_t
{
    NoProtected = 0,
    Protected = 1
};

enum class CalibrationResult : uint8_t
{
    Calibrating = 0,
    CalibratedSuccess = 1,
    CalibratingFail = 2
};

enum class WorkMode : uint8_t
{
    CrOpen = 0,
    CrClose = 1,
    CrvFoc = 2,
    SrOpen = 3,
    SrClose = 4,
    SrvFoc = 5
};

enum class HoldingStrength : uint8_t
{
    TEN_PERCENT = 0,
    TWENTLY_PERCENT = 1,
    THIRTY_PERCENT = 2,
    FOURTY_PERCENT = 3,
    FIFTHTY_PERCENT = 4,
    SIXTY_PERCENT = 5,
    SEVENTY_PERCENT = 6,
    EIGHTY_PERCENT = 7,
    NIGHTY_PERCENT = 8
};

enum class EnPinEnable : uint8_t
{
    ActiveLow = 0,
    ActiveHigh = 1,
    ActiveAlways = 2
};

enum class CanBitrate : uint8_t
{
    Rate125K = 0,
    Rate250K = 1,
    Rate500K = 2,
    Rate1M = 3
};

enum class EndStopLevel : uint8_t
{
    Low = 0,
    High = 1
};

enum class GoHomeResult : uint8_t
{
    Fail = 0,
    Start = 1,
    Success = 2,
};

enum class Mode0 : uint8_t
{
    Disable = 0,
    DirMode = 1,
    NearMode = 2
};

enum class SaveCleanState : uint8_t
{
    Save = 0xC8,
    Clean = 0xCA
};

enum class RunMotorResult : uint8_t
{
    RunFail = 0,
    RunStarting = 1,
    RunComplete = 2,
    RunEndLimitStoped = 3
};

enum class MotorStatus : uint8_t
{
    Fail = 0,
    MotorStop = 1,
    MotorSpeedUp = 2,
    MotorSpeedDown = 3,
    MotorFullSpeed = 4,
    MotorHoming = 5,
    MotorIsCalibrating = 6
};

enum class MotorShaftProtectionStatus : uint8_t
{
    Protected = 1,
    NotProtected = 0
};

#endif // MKS_ENUMS_HPP