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

#include "roboeyes_display.h"
#include "wifi_manager.h"
#include "mqtt_handler.h"

void setup()
{
  // Disable I2C logs
  esp_log_level_set("i2c.master", ESP_LOG_NONE);

  Serial.begin(SystemConfig::SERIAL_BAUD_RATE);

  // Init hardware
  indicators.begin();
  initOLED();
  ServoController::begin();
  initRoboEyes();
  speaker.begin();

  // Play a WAV file stored in LittleFS (16-bit PCM, 44.1 kHz)
  speaker.listFiles();
  speaker.playWav("/start_speech.wav");

  // Connect WiFi + MQTT
  WiFi.mode(WIFI_STA);
  WiFi.begin(NetworkConfig::WIFI_SSID, NetworkConfig::WIFI_PASSWORD);
  
  Serial.print("Connecting to WiFi");
  int attempts = 20;
  while (WiFi.status() != WL_CONNECTED && attempts-- > 0) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi connection failed");
  }
  indicators.blink(Indicators::LED::STATUS, 3, 200); // Blink status LED 3 times
  indicators.set(Indicators::LED::STATUS, true);
  speaker.playWav("/connected_to_wifi.wav");

  delay(100);

  if(setupMQTT())
  {
    Serial.println("MQTT connected.");
    speaker.playWav("/connected_to_mqtt.wav");
    indicators.blink(Indicators::LED::NETWORK, 3, 200); // Blink network LED 3 times
    indicators.set(Indicators::LED::NETWORK, true);
  }
  else
  {
    Serial.println("MQTT connection failed.");
  }

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

  sendSensorData();
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
  ServoController::update();

  // Update microphone and LED status
  mic.update();

  // --- WiFi + MQTT handling ---
  if (WiFi.status() != WL_CONNECTED) {
    static unsigned long lastReconnectAttempt = 0;
    unsigned long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      Serial.println("WiFi disconnected. Attempting to reconnect...");
      WiFi.reconnect();
      lastReconnectAttempt = now;
    }
  }
  handleMQTT();

  // Publish sensor values periodically
  if (now - lastMqttMessage >= mqttInterval)
  {
    sendSensorData();
  }
}

void sendSensorData()
{
  unsigned long now = millis();
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

  indicators.blink(Indicators::LED::ACTIVITY, 3, 200); // Blink activity LED 3 times

  lastMqttMessage = now;

  float currentVolume = mic.getVolume();
  Serial.println("Published sensor data. Current volume: " + String(currentVolume));
}