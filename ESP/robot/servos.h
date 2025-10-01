#ifndef SERVOS_H
#define SERVOS_H

#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>
#include <array>

// Robot joint configuration
enum class Joint {
    ROOT = 0,
    ARM_A1,    // Left arm
    ARM_A2,    // Right arm (mirrors A1)
    ARM_B,     // Common arm segment
    WRIST_A,   // Wrist rotation
    WRIST_B,   // Wrist tilt
    GRIPPER,   // End effector
    COUNT      // Total number of joints
};

class ServoController {
public:
    // Initialize the servo controller
    static bool begin();
    
    // Update servo positions (call in main loop)
    static void update();
    
    // Control methods
    static void setTargetAngle(Joint joint, float angle);
    static void setSpeed(Joint joint, float speed);
    
    // Status methods
    static float getCurrentAngle(Joint joint);
    static bool isMoving(Joint joint);
    static bool atTarget(Joint joint);

private:
    static constexpr uint16_t PWM_MIN = 150;    // Minimum PWM value (0 degrees)
    static constexpr uint16_t PWM_MAX = 600;    // Maximum PWM value (180 degrees)
    static constexpr uint8_t PWM_FREQ = 50;     // Standard servo frequency (Hz)
    
    // Joint limits and configuration
    struct JointConfig {
        float minAngle;     // Minimum allowed angle
        float maxAngle;     // Maximum allowed angle
        float defaultSpeed; // Default movement speed
        bool isReversed;   // Whether servo is mounted in reverse
    };
    
    static const std::array<JointConfig, static_cast<size_t>(Joint::COUNT)> JOINT_CONFIGS;
    
    // Current state
    static std::array<float, static_cast<size_t>(Joint::COUNT)> currentAngles;
    static std::array<float, static_cast<size_t>(Joint::COUNT)> targetAngles;
    static std::array<float, static_cast<size_t>(Joint::COUNT)> speeds;
    
    // Helper methods
    static uint16_t angleToPWM(float angle);
    static bool isValidJoint(Joint joint);
    static void updateJoint(Joint joint);
};

#endif // SERVOS_H
