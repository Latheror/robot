#include "servos.h"
#include <algorithm>

// Static member initialization
static Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// Initialize static members with default values
std::array<float, static_cast<size_t>(Joint::COUNT)> ServoController::currentAngles = {90, 90, 90, 90, 90, 90, 90};
std::array<float, static_cast<size_t>(Joint::COUNT)> ServoController::targetAngles = {90, 90, 90, 90, 90, 90, 90};
std::array<float, static_cast<size_t>(Joint::COUNT)> ServoController::speeds = {0.1, 0.1, 0.1, 0.5, 0.7, 0.7, 1.0};

// Joint configuration data
const std::array<ServoController::JointConfig, static_cast<size_t>(Joint::COUNT)> ServoController::JOINT_CONFIGS = {{
    {70,  110, 0.1, false}, // ROOT
    {80,  100, 0.1, false}, // ARM_A1
    {80,  100, 0.1, true},  // ARM_A2 (mirrored)
    {80,  100, 0.5, false}, // ARM_B
    {0,   180, 0.7, false}, // WRIST_A
    {0,   180, 0.7, false}, // WRIST_B
    {80,  100, 1.0, false}  // GRIPPER
}};

bool ServoController::begin() {
    pwm.begin();
    pwm.setPWMFreq(PWM_FREQ);
    
    // Initialize all servos to their current positions
    for (size_t i = 0; i < static_cast<size_t>(Joint::COUNT); i++) {
        updateJoint(static_cast<Joint>(i));
    }
    
    return true;
}

void ServoController::update() {
    for (size_t i = 0; i < static_cast<size_t>(Joint::COUNT); i++) {
        Joint joint = static_cast<Joint>(i);
        if (!atTarget(joint)) {
            updateJoint(joint);
        }
    }
}

void ServoController::setTargetAngle(Joint joint, float angle) {
    if (!isValidJoint(joint)) return;
    
    size_t idx = static_cast<size_t>(joint);
    const auto& config = JOINT_CONFIGS[idx];
    
    // Clamp angle to valid range
    angle = std::clamp(angle, config.minAngle, config.maxAngle);
    targetAngles[idx] = angle;
    
    Serial.printf("[SERVO] Joint %d target: %.1f°\n", idx, angle);
}

void ServoController::setSpeed(Joint joint, float speed) {
    if (isValidJoint(joint) && speed > 0) {
        speeds[static_cast<size_t>(joint)] = speed;
    }
}

float ServoController::getCurrentAngle(Joint joint) {
    return isValidJoint(joint) ? currentAngles[static_cast<size_t>(joint)] : -1;
}

bool ServoController::isMoving(Joint joint) {
    return !atTarget(joint);
}

bool ServoController::atTarget(Joint joint) {
    if (!isValidJoint(joint)) return true;
    size_t idx = static_cast<size_t>(joint);
    return fabs(targetAngles[idx] - currentAngles[idx]) <= 0.01;
}

uint16_t ServoController::angleToPWM(float angle) {
    return map(angle, 0, 180, PWM_MIN, PWM_MAX);
}

bool ServoController::isValidJoint(Joint joint) {
    return static_cast<size_t>(joint) < static_cast<size_t>(Joint::COUNT);
}

void ServoController::updateJoint(Joint joint) {
    size_t idx = static_cast<size_t>(joint);
    const auto& config = JOINT_CONFIGS[idx];
    
    // Update position with smooth interpolation
    float error = targetAngles[idx] - currentAngles[idx];
    if (fabs(error) > 0.01) {
        float step = speeds[idx] * (error > 0 ? 1 : -1);
        currentAngles[idx] += step;
        
        // Ensure we don't overshoot
        if ((step > 0 && currentAngles[idx] > targetAngles[idx]) ||
            (step < 0 && currentAngles[idx] < targetAngles[idx])) {
            currentAngles[idx] = targetAngles[idx];
        }
        
        // Apply to hardware
        float angle = config.isReversed ? (180 - currentAngles[idx]) : currentAngles[idx];
        pwm.setPWM(idx, 0, angleToPWM(angle));
    }
}
