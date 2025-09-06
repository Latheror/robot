#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "wifi.h"
#include "mqtt_handler.h"
#include "oled_display.h"
#include "roboeyes_display.h"

// Servo driver
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// Servo defs
#define SERVO_ROOT    0
#define SERVO_ARM_A1  1
#define SERVO_ARM_A2  2
#define SERVO_ARM_B   3
#define SERVO_WRIST_A 4
#define SERVO_WRIST_B 5
#define SERVO_GRIPPER 6

#define SERVOMIN 150
#define SERVOMAX 600

float currentAngles[6] = {90, 90, 90, 90, 90, 90};
float targetAngles[6]  = {90, 90, 90, 90, 90, 90};
float speedFactors[6]  = {1.0, 0.5, 0.5, 0.7, 0.7, 1.0};

unsigned long lastMqttMessage = 0;
const unsigned long mqttInterval = 60 * 1000;

// ⚡ Timer for RoboEyes
unsigned long lastEyesUpdate = 0;
const unsigned long eyesInterval = 10; // ~100 FPS

void setup() {
  Serial.begin(9600);
  Wire.begin();
  pwm.begin();
  pwm.setPWMFreq(50);

  // Init display + RoboEyes
  initOLED();
  initRoboEyes();

  // Connect WiFi + MQTT
  setupWiFi();
  delay(100);
  setupMQTT();
}

void loop() {
  unsigned long now = millis();

  // --- RoboEyes independent update ---
  if (now - lastEyesUpdate >= eyesInterval) {
    handleRoboEyes();
    lastEyesUpdate = now;
  }

  // --- Servo handling ---
  if (Serial.available() >= 6) {
    for (int i = 0; i < 6; i++) {
      targetAngles[i] = Serial.parseInt();
    }
  }

  for (int i = 0; i < 6; i++) {
    if (abs(targetAngles[i] - currentAngles[i]) > 0.01) {
      if (currentAngles[i] < targetAngles[i]) {
        currentAngles[i] += speedFactors[i];
        if (currentAngles[i] > targetAngles[i]) currentAngles[i] = targetAngles[i];
      } else if (currentAngles[i] > targetAngles[i]) {
        currentAngles[i] -= speedFactors[i];
        if (currentAngles[i] < targetAngles[i]) currentAngles[i] = targetAngles[i];
      }
    }
  }

  pwm.setPWM(SERVO_ROOT,    0, map(currentAngles[0], 0, 180, SERVOMIN, SERVOMAX));
  pwm.setPWM(SERVO_ARM_A1,  0, map(currentAngles[1], 0, 180, SERVOMIN, SERVOMAX));
  pwm.setPWM(SERVO_ARM_A2,  0, map(180 - currentAngles[1], 0, 180, SERVOMIN, SERVOMAX));
  pwm.setPWM(SERVO_ARM_B,   0, map(currentAngles[2], 0, 180, SERVOMIN, SERVOMAX));
  pwm.setPWM(SERVO_WRIST_A, 0, map(currentAngles[3], 0, 180, SERVOMIN, SERVOMAX));
  pwm.setPWM(SERVO_WRIST_B, 0, map(currentAngles[4], 0, 180, SERVOMIN, SERVOMAX));
  pwm.setPWM(SERVO_GRIPPER, 0, map(currentAngles[5], 0, 180, SERVOMIN, SERVOMAX));

  // --- WiFi + MQTT handling ---
  checkWiFiConnection();
  handleMQTT();

  if (now - lastMqttMessage >= mqttInterval) {
    char sensorMsg[256];
    float temperature = 22.5 + (random(-100, 100) / 100.0);
    float humidity = 45.0 + (random(-50, 50) / 10.0);
    int light = random(800, 1000);

    snprintf(sensorMsg, sizeof(sensorMsg),
             "{"
             "\"timestamp\":%lu,"
             "\"sensors\":{"
               "\"temperature\":%.2f,"
               "\"humidity\":%.1f,"
               "\"light\":%d"
             "},"
             "\"units\":{"
               "\"temperature\":\"celsius\","
               "\"humidity\":\"percent\","
               "\"light\":\"lux\""
             "}"
             "}",
             now, temperature, humidity, light);

    publishMessage(MQTT_TOPIC_SENSORS, sensorMsg);
    lastMqttMessage = now;
  }
}
