#include "servos.h"
#include <algorithm>

// Static member definitions
Adafruit_PWMServoDriver ServoController::pwm = Adafruit_PWMServoDriver();

// Physical servo mapping (board channels)
std::array<ServoInfo, 7> ServoController::servos = {{
    {0}, // ROOT
    {1}, // ARM_A1
    {2}, // ARM_A2 (mirror)
    {3}, // ARM_B
    {4}, // WRIST_A
    {5}, // WRIST_B
    {6}  // GRIPPER
}};

// Logical joint configuration
std::array<JointInfo, static_cast<size_t>(Joint::COUNT)> ServoController::joints = {{
    {70, 110, 0.5, {0, -1}, 1},  // ROOT → 1 servo
    {40, 100, 0.6, {1, 2}, 2},   // ARM_A → mirrors servo 2
    {40, 100, 0.5, {3, -1}, 1},  // ARM_B
    {80, 130, 0.7, {4, -1}, 1},   // WRIST_A
    {0, 180, 0.7, {5, -1}, 1},   // WRIST_B
    {60, 120, 1.0, {6, -1}, 1}   // GRIPPER
}};

// Servo states
std::array<float, static_cast<size_t>(Joint::COUNT)> ServoController::currentAngles = {90, 90, 90, 90, 90, 90};
std::array<float, static_cast<size_t>(Joint::COUNT)> ServoController::targetAngles = {90, 90, 90, 90, 90, 90};

// -------------------------------------------------------

bool ServoController::begin() {
    Serial.println("[SERVO] Initializing servo controller...");
    pwm.begin();
    pwm.setPWMFreq(PWM_FREQ);

    for (size_t i = 0; i < static_cast<size_t>(Joint::COUNT); i++) {
        updateJoint(static_cast<Joint>(i));
    }

    Serial.println("[SERVO] Initialization complete");
    return true;
}

// -------------------------------------------------------

void ServoController::update() {
    for (size_t i = 0; i < static_cast<size_t>(Joint::COUNT); i++) {
        Joint joint = static_cast<Joint>(i);
        if (!atTarget(joint)) {
            updateJoint(joint);
        }
    }
}

// -------------------------------------------------------

void ServoController::setTargetAngle(Joint joint, float angle) {
    size_t idx = static_cast<size_t>(joint);
    if (idx >= joints.size()) {
        Serial.print("[ERROR] Invalid joint index: ");
        Serial.println(idx);
        return;
    }

    const auto& config = joints[idx];
    float clamped = std::clamp(angle, config.minAngle, config.maxAngle);
    targetAngles[idx] = clamped;

    Serial.print("[SERVO] Joint ");
    Serial.print(idx);
    Serial.print(" target angle set to ");
    Serial.println(clamped);
}

// -------------------------------------------------------

float ServoController::getCurrentAngle(Joint joint) {
    return isValidJoint(joint) ? currentAngles[static_cast<size_t>(joint)] : -1;
}

// -------------------------------------------------------

bool ServoController::isMoving(Joint joint) {
    return !atTarget(joint);
}

// -------------------------------------------------------

bool ServoController::atTarget(Joint joint) {
    if (!isValidJoint(joint)) return true;
    size_t idx = static_cast<size_t>(joint);
    return fabs(targetAngles[idx] - currentAngles[idx]) <= 0.01;
}

// -------------------------------------------------------

uint16_t ServoController::angleToPWM(float angle) {
    return map((int)angle, 0, 180, PWM_MIN, PWM_MAX);
}

// -------------------------------------------------------

bool ServoController::isValidJoint(Joint joint) {
    return static_cast<size_t>(joint) < static_cast<size_t>(Joint::COUNT);
}

// -------------------------------------------------------

void ServoController::updateJoint(Joint joint) {

    Serial.print("[SERVO] Updating joint ");
    Serial.println(static_cast<size_t>(joint));

    size_t idx = static_cast<size_t>(joint);
    if (idx >= joints.size()) return;

    const auto& config = joints[idx];
    float error = targetAngles[idx] - currentAngles[idx];

    if (fabs(error) <= 0.01) return;

    // Smooth motion step
    float step = config.speed * (error > 0 ? 1 : -1);
    currentAngles[idx] += step;

    if ((step > 0 && currentAngles[idx] > targetAngles[idx]) ||
        (step < 0 && currentAngles[idx] < targetAngles[idx])) {
        currentAngles[idx] = targetAngles[idx];
    }

    // Apply to each physical servo
    for (uint8_t i = 0; i < config.servoCount; i++) {
        int8_t servoIndex = config.servos[i];
        if (servoIndex < 0 || (size_t)servoIndex >= servos.size()) continue;

        const auto& servo = servos[servoIndex];
        float angle = (i == 0) ? currentAngles[idx] : 180 - currentAngles[idx];

        pwm.setPWM(servo.pwmIndex, 0, angleToPWM(angle));

        // Trace
        // Serial.print("[SERVO] Joint ");
        // Serial.print(idx);
        // Serial.print(" → Servo #");
        // Serial.print(servoIndex);
        // Serial.print(" (PWM ");
        // Serial.print(servo.pwmIndex);
        // Serial.print(") = ");
        // Serial.print(angle);
        // Serial.println("°");
    }
}
