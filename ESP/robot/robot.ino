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

// Timer for periodic MQTT messages
unsigned long lastMqttMessage = 0;
const unsigned long mqttInterval = (1000 * 60); // Send message every minute

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
  
  // Send periodic sensor message every 2 seconds
  unsigned long currentTime = millis();
  if (currentTime - lastMqttMessage >= mqttInterval) {
    char sensorMsg[256];
    
    // Simulate some sensor data
    float temperature = 22.5 + (random(-100, 100) / 100.0); // Random temp between 21.5 and 23.5°C
    float humidity = 45.0 + (random(-50, 50) / 10.0);      // Random humidity between 40-50%
    int light = random(800, 1000);                         // Random light level
    
    // Create JSON message with sensor data
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
             currentTime, temperature, humidity, light);
             
    publishMessage(MQTT_TOPIC_SENSORS, sensorMsg);
    lastMqttMessage = currentTime;
  }
}
