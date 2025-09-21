#ifndef SERVOS_H
#define SERVOS_H

#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>

// --- Servo definitions ---
#define SERVO_ROOT    0
#define SERVO_ARM_A1  1
#define SERVO_ARM_A2  2
#define SERVO_ARM_B   3
#define SERVO_WRIST_A 4
#define SERVO_WRIST_B 5
#define SERVO_GRIPPER 6

#define SERVOMIN 150
#define SERVOMAX 600

// Number of independent servos (ARM_A2 mirrors A1)
#define NUM_SERVOS 6  

// --- Public functions ---
void initServos();
void updateServos();
void setTargetAngle(uint8_t servoIndex, float angle);
float getCurrentAngle(uint8_t servoIndex);
void setServoSpeed(uint8_t servoIndex, float speed);

#endif
