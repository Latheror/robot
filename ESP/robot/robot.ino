#include <Wire.h>
#include "wifi_manager.h"
#include "mqtt_handler.h"
#include "oled_display.h"
#include "roboeyes_display.h"
#include "esp_log.h"
#include "servos.h"
#include "speaker.h"
#include "indicators.h"
#include "INMP441.h"

unsigned long lastMqttMessage = 0;
const unsigned long mqttInterval = 60 * 1000;

// Timer for RoboEyes
unsigned long lastEyesUpdate = 0;
const unsigned long eyesInterval = 10; // ~100 FPS

Speaker speaker;
Indicators indicators;
INMP441 mic;

void setup()
{
  // Disable I2C logs
  esp_log_level_set("i2c.master", ESP_LOG_NONE);

  Serial.begin(115200);

  // Init hardware
  indicators.init();
  initOLED();
  initServos();
  initRoboEyes();
  speaker.init();

  // Play a WAV file stored in LittleFS (16-bit PCM, 44.1 kHz)
  speaker.listFiles();
  speaker.playWav("/start_speech.wav");

  // Connect WiFi + MQTT
  setupWiFi();
  indicators.blinkLED(1, 3, 200); // Blink LED1 3 times
  indicators.setLED(1, true);
  speaker.playWav("/connected_to_wifi.wav");


  delay(100);

  setupMQTT();
  indicators.blinkLED(2, 3, 200); // Blink LED1 3 times
  indicators.setLED(2, true);

  speaker.playExampleSound();

  // Initialize INMP441 microphone
  if (mic.begin())
  {
    Serial.println("Microphone ready.");
  }
  else
  {
    Serial.println("Microphone initialization failed.");
  }
}

void loop()
{

  unsigned long now = millis();

  // --- RoboEyes independent update ---
  if (now - lastEyesUpdate >= eyesInterval)
  {
    handleRoboEyes();
    lastEyesUpdate = now;
  }

  // Update servo positions smoothly
  updateServos();

  // --- WiFi + MQTT handling ---
  checkWiFiConnection();
  handleMQTT();

  // Publish sensor values periodically
  if (now - lastMqttMessage >= mqttInterval)
  {
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

    indicators.blinkLED(3, 3, 200); // Blink LED1 3 times

    lastMqttMessage = now;

    int32_t sample = mic.readSample();
    Serial.println("Published sensor data:");
    Serial.println(sample);
  }
}
