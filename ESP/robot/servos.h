#ifndef SERVOS_H
#define SERVOS_H

#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>
#include <array>

// Logical robot joints
enum class Joint {
    ROOT = 0,
    ARM_A,
    ARM_B,
    WRIST_A,
    WRIST_B,
    GRIPPER,
    COUNT
};

// Physical servo info
struct ServoInfo {
    uint8_t pwmIndex; // PWM channel on the board
};

// Logical joint configuration
struct JointInfo {
    float minAngle;
    float maxAngle;
    float speed;
    std::array<int8_t, 2> servos; // Up to 2 physical servos per joint (2nd can be mirrored)
    uint8_t servoCount;           // How many of these are valid
};

class ServoController {
public:
    static bool begin();
    static void update();
    static bool isInitialized() { return _initialized; }

    static void setTargetAngle(Joint joint, float angle);

    static float getCurrentAngle(Joint joint);
    static bool isMoving(Joint joint);
    static bool atTarget(Joint joint);

private:
    static constexpr uint16_t PWM_MIN = 150; // 0° pulse
    static constexpr uint16_t PWM_MAX = 600; // 180° pulse
    static constexpr uint8_t  PWM_FREQ = 50; // 50Hz for servos

    static Adafruit_PWMServoDriver pwm;

    // Physical servos (7 total)
    static std::array<ServoInfo, 7> servos;

    // Logical joints (6 total)
    static std::array<JointInfo, static_cast<size_t>(Joint::COUNT)> joints;

    // State tracking
    static std::array<float, static_cast<size_t>(Joint::COUNT)> currentAngles;
    static std::array<float, static_cast<size_t>(Joint::COUNT)> targetAngles;
    static std::array<float, static_cast<size_t>(Joint::COUNT)> speeds;
    static bool _initialized;

    // Internal helpers
    static void updateJoint(Joint joint);
    static uint16_t angleToPWM(float angle);
    static bool isValidJoint(Joint joint);
};

#endif // SERVOS_H
