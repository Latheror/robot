#include "servos.h"

// PCA9685 driver instance
static Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// Servo state arrays
static float currentAngles[NUM_JOINTS] = {90, 90, 90, 90, 90, 90};
static float targetAngles[NUM_JOINTS]  = {90, 90, 90, 90, 90, 90};
static float speedFactors[NUM_JOINTS]  = {0.1, 0.1, 0.5, 0.7, 0.7, 1.0};

// Fixed min/max angle limits per joint
// (tune these values to the robot’s safe ranges)
static const float minAngles[NUM_JOINTS] = {70, 80, 80, 0, 0, 80};
static const float maxAngles[NUM_JOINTS] = {110, 100, 100, 180, 180, 100};

void initServos() {
    pwm.begin();
    pwm.setPWMFreq(50); // Standard servo frequency
}

void setTargetAngle(uint8_t servoIndex, float angle) {

    Serial.print("Set target angle for servo ");
    Serial.print(servoIndex);
    Serial.print(": ");
    Serial.println(angle);

    if (servoIndex < NUM_JOINTS) {
        // Clamp between fixed limits
        if (angle < minAngles[servoIndex]) angle = minAngles[servoIndex];
        if (angle > maxAngles[servoIndex]) angle = maxAngles[servoIndex];
        targetAngles[servoIndex] = angle;
    }
}

float getCurrentAngle(uint8_t servoIndex) {
    if (servoIndex < NUM_JOINTS) {
        return currentAngles[servoIndex];
    }
    return -1; // invalid index
}

void setServoSpeed(uint8_t servoIndex, float speed) {
    if (servoIndex < NUM_JOINTS && speed > 0) {
        speedFactors[servoIndex] = speed;
    }
}

void updateServos() {
    // Smooth interpolation for joints
    for (int i = 0; i < NUM_JOINTS; i++) {
        if (fabs(targetAngles[i] - currentAngles[i]) > 0.01) {
            if (currentAngles[i] < targetAngles[i]) {
                currentAngles[i] += speedFactors[i];
                if (currentAngles[i] > targetAngles[i]) currentAngles[i] = targetAngles[i];
            } else {
                currentAngles[i] -= speedFactors[i];
                if (currentAngles[i] < targetAngles[i]) currentAngles[i] = targetAngles[i];
            }
        }
    }

    // Apply PWM to all physical servos
    pwm.setPWM(SERVO_ROOT,    0, map(currentAngles[0], 0, 180, SERVOMIN, SERVOMAX));
    pwm.setPWM(SERVO_ARM_A1,  0, map(currentAngles[1], 0, 180, SERVOMIN, SERVOMAX));
    pwm.setPWM(SERVO_ARM_A2,  0, map(180 - currentAngles[1], 0, 180, SERVOMIN, SERVOMAX)); // miroir
    pwm.setPWM(SERVO_ARM_B,   0, map(currentAngles[2], 0, 180, SERVOMIN, SERVOMAX));
    pwm.setPWM(SERVO_WRIST_A, 0, map(currentAngles[3], 0, 180, SERVOMIN, SERVOMAX));
    pwm.setPWM(SERVO_WRIST_B, 0, map(currentAngles[4], 0, 180, SERVOMIN, SERVOMAX));
    pwm.setPWM(SERVO_GRIPPER, 0, map(currentAngles[5], 0, 180, SERVOMIN, SERVOMAX));
}