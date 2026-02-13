#include "servos.h"
#include "indicators.h"

extern Indicators indicators;  // External variable declaration

// Static member definitions
Adafruit_PWMServoDriver ServoController::pwm = Adafruit_PWMServoDriver();
bool ServoController::_initialized = false;

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
    if (!pwm.begin()) {
        Serial.println("[SERVO] Failed to initialize PCA9685! Robot arm control unavailable");
        _initialized = false;
        return false;
    }
    
    pwm.setPWMFreq(PWM_FREQ);

    for (size_t i = 0; i < static_cast<size_t>(Joint::COUNT); i++) {
        updateJoint(static_cast<Joint>(i));
    }

    _initialized = true;
    Serial.println("[SERVO] Initialization complete - Robot arm ready");
    return true;
}

// -------------------------------------------------------

void ServoController::update() {
    if (!_initialized) {
        Serial.println("[SERVO] Controller not initialized, skipping update");
        return;
    }

    static bool wasMoving = false;
    bool isAnyJointMoving = false;
    
    for (size_t i = 0; i < static_cast<size_t>(Joint::COUNT); i++) {
        Joint joint = static_cast<Joint>(i);
        if (!atTarget(joint)) {
            updateJoint(joint);
            isAnyJointMoving = true;
        }
    }

    // Update LED based on movement status
    if (isAnyJointMoving != wasMoving) {
        wasMoving = isAnyJointMoving;
        
        if (isAnyJointMoving) {
            // Blink green LED while servos are moving
            Serial.println("[SERVO] Movement started - LED blinking");
            indicators.blink(Indicators::LED_PINS::MOTORS_MOVING, 1, 100);
        } else {
            // Solid green when servos are stopped
            Serial.println("[SERVO] Movement stopped - LED solid");
            indicators.setColor(Indicators::LED_PINS::MOTORS_MOVING, 0, 255, 0);
        }
    }
}

// -------------------------------------------------------

void ServoController::setTargetAngle(Joint joint, float angle) {
    if (!_initialized) {
        Serial.println("[SERVO] Controller not initialized, ignoring setTargetAngle");
        return;
    }

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

/**
 * @brief Converts angle to PWM value.
 * @param angle The angle in degrees.
 * @return The PWM value.
 */
uint16_t ServoController::angleToPWM(float angle) {
    return map((int)angle, 0, 180, PWM_MIN, PWM_MAX);
}

// -------------------------------------------------------

/**
 * @brief Validates a joint index.
 * @param joint The joint to validate.
 * @return true if valid, false otherwise.
 */
bool ServoController::isValidJoint(Joint joint) {
    return static_cast<size_t>(joint) < static_cast<size_t>(Joint::COUNT);
}

// -------------------------------------------------------

/**
 * @brief Updates a specific joint.
 * @param joint The joint to update.
 */
void ServoController::updateJoint(Joint joint) {

    //Serial.print("[SERVO] Updating joint ");
    //Serial.println(static_cast<size_t>(joint));

    size_t idx = static_cast<size_t>(joint);
    if (idx >= ServoController::joints.size()) return;

    const auto& config = ServoController::joints[idx];
    float error = ServoController::targetAngles[idx] - ServoController::currentAngles[idx];

    if (fabs(error) <= 0.01) return;

    // Smooth motion step
    float step = config.speed * (error > 0 ? 1 : -1);
    ServoController::currentAngles[idx] += step;

    if ((step > 0 && ServoController::currentAngles[idx] > ServoController::targetAngles[idx]) ||
        (step < 0 && ServoController::currentAngles[idx] < ServoController::targetAngles[idx])) {
        ServoController::currentAngles[idx] = ServoController::targetAngles[idx];
    }

    // Apply to each physical servo
    for (uint8_t i = 0; i < config.servoCount; i++) {
        int8_t servoIndex = config.servos[i];
        if (servoIndex < 0 || (size_t)servoIndex >= ServoController::servos.size()) continue;

        const auto& servo = ServoController::servos[servoIndex];
        float angle = (i == 0) ? ServoController::currentAngles[idx] : 180 - ServoController::currentAngles[idx];

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
