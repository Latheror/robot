#include <Wire.h>
#include "wifi_manager.h"
#include "mqtt_handler.h"
#include "oled_display.h"
#include "roboeyes_display.h"
#include "esp_log.h"
#include "servos.h"
#include "speaker.h"

unsigned long lastMqttMessage = 0;
const unsigned long mqttInterval = 10 * 1000;

// Timer for RoboEyes
unsigned long lastEyesUpdate = 0;
const unsigned long eyesInterval = 10; // ~100 FPS

Speaker speaker;

void setup() {
  // Disable I2C logs
  esp_log_level_set("i2c.master", ESP_LOG_NONE);

  Serial.begin(115200);

  // Init hardware
  initServos();
  initOLED();
  initRoboEyes();
  speaker.init();               // All configuration handled inside Speaker

  // Connect WiFi + MQTT
  setupWiFi();
  delay(100);
  setupMQTT();

  speaker.playExampleSound();
}

void loop() {
  unsigned long now = millis();

  // --- RoboEyes independent update ---
  if (now - lastEyesUpdate >= eyesInterval) {
    handleRoboEyes();
    lastEyesUpdate = now;
  }

  // --- Servo handling: receive new target angles from Serial ---
  if (Serial.available() >= NUM_JOINTS) {
    for (int i = 0; i < NUM_JOINTS; i++) {
      setTargetAngle(i, Serial.parseInt());
    }
  }

  // Update servo positions smoothly
  updateServos();

  // --- WiFi + MQTT handling ---
  checkWiFiConnection();
  handleMQTT();

  // Publish sensor values periodically
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
