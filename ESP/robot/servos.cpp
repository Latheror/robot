#include "servos.h"

// PCA9685 driver instance
static Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// Servo state arrays
static float currentAngles[NUM_SERVOS] = {90, 90, 90, 90, 90, 90};
static float targetAngles[NUM_SERVOS]  = {90, 90, 90, 90, 90, 90};
static float speedFactors[NUM_SERVOS]  = {1.0, 0.5, 0.5, 0.7, 0.7, 1.0};

void initServos() {
    pwm.begin();
    pwm.setPWMFreq(50); // Standard servo frequency
}

void setTargetAngle(uint8_t servoIndex, float angle) {
    if (servoIndex < NUM_SERVOS) {
        if (angle < 0) angle = 0;
        if (angle > 180) angle = 180;
        targetAngles[servoIndex] = angle;
    }
}

float getCurrentAngle(uint8_t servoIndex) {
    if (servoIndex < NUM_SERVOS) {
        return currentAngles[servoIndex];
    }
    return -1; // invalid index
}

void setServoSpeed(uint8_t servoIndex, float speed) {
    if (servoIndex < NUM_SERVOS && speed > 0) {
        speedFactors[servoIndex] = speed;
    }
}

void updateServos() {
    // Smooth angle interpolation
    for (int i = 0; i < NUM_SERVOS; i++) {
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

    // Apply PWM values
    pwm.setPWM(SERVO_ROOT,    0, map(currentAngles[0], 0, 180, SERVOMIN, SERVOMAX));
    pwm.setPWM(SERVO_ARM_A1,  0, map(currentAngles[1], 0, 180, SERVOMIN, SERVOMAX));
    pwm.setPWM(SERVO_ARM_A2,  0, map(180 - currentAngles[1], 0, 180, SERVOMIN, SERVOMAX)); // mirrored
    pwm.setPWM(SERVO_ARM_B,   0, map(currentAngles[2], 0, 180, SERVOMIN, SERVOMAX));
    pwm.setPWM(SERVO_WRIST_A, 0, map(currentAngles[3], 0, 180, SERVOMIN, SERVOMAX));
    pwm.setPWM(SERVO_WRIST_B, 0, map(currentAngles[4], 0, 180, SERVOMIN, SERVOMAX));
    pwm.setPWM(SERVO_GRIPPER, 0, map(currentAngles[5], 0, 180, SERVOMIN, SERVOMAX));
}
