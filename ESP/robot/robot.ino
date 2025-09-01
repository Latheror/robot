#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "wifi.h"
#include "mqtt_handler.h"

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// Define servos
#define SERVO_ROOT    0
#define SERVO_ARM_A1  1
#define SERVO_ARM_A2  2
#define SERVO_ARM_B   3
#define SERVO_WRIST_A 4
#define SERVO_WRIST_B 5
#define SERVO_GRIPPER 6

// Safe pulse range for your servos
#define SERVOMIN  150
#define SERVOMAX  600

// Initial servo positions
float currentAngles[6] = {90, 90, 90, 90, 90, 90};
float targetAngles[6] = {90, 90, 90, 90, 90, 90};

// Speed factors (degrees per step, larger = faster)
float speedFactors[6] = {1.0, 0.5, 0.5, 0.7, 0.7, 1.0};  

void setup() {
  Serial.begin(9600);
  Wire.begin();
  pwm.begin();
  pwm.setPWMFreq(50); // Standard servo frequency
  
  // Connect to WiFi
  setupWiFi();
  delay(100);
  
  // Setup MQTT
  setupMQTT();
}

void loop() {
  // Read new target angles from Serial
  if (Serial.available() >= 6) {
    for (int i = 0; i < 6; i++) {
      targetAngles[i] = Serial.parseInt();
    }
  }

  // Move servos smoothly towards target
  bool moving = false;
  for (int i = 0; i < 6; i++) {
    if (abs(targetAngles[i] - currentAngles[i]) > 0.01) {
      moving = true;
      if (currentAngles[i] < targetAngles[i]) {
        currentAngles[i] += speedFactors[i];
        if (currentAngles[i] > targetAngles[i]) currentAngles[i] = targetAngles[i];
      } else if (currentAngles[i] > targetAngles[i]) {
        currentAngles[i] -= speedFactors[i];
        if (currentAngles[i] < targetAngles[i]) currentAngles[i] = targetAngles[i];
      }
    }
  }

  // Update all servos
  pwm.setPWM(SERVO_ROOT,    0, map(currentAngles[0], 0, 180, SERVOMIN, SERVOMAX));
  pwm.setPWM(SERVO_ARM_A1,  0, map(currentAngles[1], 0, 180, SERVOMIN, SERVOMAX));
  pwm.setPWM(SERVO_ARM_A2,  0, map(180 - currentAngles[1], 0, 180, SERVOMIN, SERVOMAX)); // Mirror
  pwm.setPWM(SERVO_ARM_B,   0, map(currentAngles[2], 0, 180, SERVOMIN, SERVOMAX));
  pwm.setPWM(SERVO_WRIST_A, 0, map(currentAngles[3], 0, 180, SERVOMIN, SERVOMAX));
  pwm.setPWM(SERVO_WRIST_B, 0, map(currentAngles[4], 0, 180, SERVOMIN, SERVOMAX));
  pwm.setPWM(SERVO_GRIPPER, 0, map(currentAngles[5], 0, 180, SERVOMIN, SERVOMAX));

  delay(20); // Adjust this for overall smoothness
  
  // Check WiFi connection periodically
  checkWiFiConnection();
  
  // Handle MQTT messages and maintain connection
  handleMQTT();
  
  // If the robot is moving, publish its position
  if (moving) {
    char positionMsg[128];
    snprintf(positionMsg, sizeof(positionMsg), 
             "{\"angles\":[%.2f,%.2f,%.2f,%.2f,%.2f,%.2f]}", 
             currentAngles[0], currentAngles[1], currentAngles[2],
             currentAngles[3], currentAngles[4], currentAngles[5]);
    publishMessage(MQTT_TOPIC_POSITION, positionMsg);
  }
}
