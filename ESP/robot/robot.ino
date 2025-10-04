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
#include "rgb_led.h"

/// <summary>Time of last MQTT message sent.</summary>
unsigned long lastMqttMessage = 0;

/// <summary>Interval between MQTT sensor publishes (ms).</summary>
const unsigned long mqttInterval = 60 * 1000;

/// <summary>Timer for RoboEyes update.</summary>
unsigned long lastEyesUpdate = 0;

/// <summary>RoboEyes update interval (~100 FPS).</summary>
const unsigned long eyesInterval = 10;

// --- Hardware modules ---
Speaker speaker;
Indicators indicators;
INMP441 mic;
RGBLed rgbLed;

/// <summary>OLED display instance.</summary>
OLEDDisplay oled;

/// <summary>RoboEyes display instance, uses the OLED display.</summary>
RoboEyesDisplay roboEyes(oled);

/// <summary>
/// Setup function: initializes hardware, WiFi, MQTT, and RoboEyes.
/// </summary>
void setup()
{
    // Disable I2C logs
    esp_log_level_set("i2c.master", ESP_LOG_NONE);

    Serial.begin(SystemConfig::SERIAL_BAUD_RATE);

    // Initialize hardware
    indicators.begin();
    oled.begin();
    ServoController::begin();
    roboEyes.begin();
    speaker.begin();

    rgbLed.begin();
    rgbLed.setBrightness(50);   // 50% brightness
    rgbLed.setColor(0, 0, 255); // blue

    // Play initial sounds
    speaker.listFiles();
    speaker.playWav("/start_speech.wav");

    // Connect WiFi
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

    indicators.blink(Indicators::LED::STATUS, 3, 200);
    indicators.set(Indicators::LED::STATUS, true);

    speaker.playWav("/connected_to_wifi.wav");
    delay(100);

    // Setup MQTT
    if (setupMQTT()) {
        Serial.println("MQTT connected.");
        speaker.playWav("/connected_to_server.wav");
        indicators.blink(Indicators::LED::NETWORK, 3, 200);
        indicators.set(Indicators::LED::NETWORK, true);
    } else {
        Serial.println("MQTT connection failed.");
    }

    // Play example sound
    speaker.playExampleSound();

    // Initialize microphone
    if (mic.begin()) {
        Serial.println("Microphone ready.");
    } else {
        Serial.println("Microphone initialization failed.");
    }

    // Send initial sensor data
    sendSensorData();
}

/// <summary>
/// Main loop: updates RoboEyes, servo positions, microphone, WiFi/MQTT, and publishes sensor data periodically.
/// </summary>
void loop()
{
    unsigned long now = millis();

    // --- RoboEyes update ---
    if (now - lastEyesUpdate >= eyesInterval) {
        roboEyes.update();
        lastEyesUpdate = now;
    }

    // --- Smooth servo updates ---
    ServoController::update();

    // --- Microphone update ---
    mic.update();

    // --- WiFi reconnect if disconnected ---
    if (WiFi.status() != WL_CONNECTED) {
        static unsigned long lastReconnectAttempt = 0;
        if (now - lastReconnectAttempt > 5000) {
            Serial.println("WiFi disconnected. Attempting to reconnect...");
            WiFi.reconnect();
            lastReconnectAttempt = now;
        }
    }

    // --- MQTT handler ---
    handleMQTT();

    // --- Publish sensor data periodically ---
    if (now - lastMqttMessage >= mqttInterval) {
        sendSensorData();
    }
}

/// <summary>
/// Gather sensor values and publish them via MQTT.
/// </summary>
void sendSensorData()
{
    unsigned long now = millis();
    char sensorMsg[256];

    float temperature = 22.5 + (random(-100, 100) / 100.0f);
    float humidity = 45.0 + (random(-50, 50) / 10.0f);
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

    // Blink activity LED
    indicators.blink(Indicators::LED::ACTIVITY, 3, 200);

    lastMqttMessage = now;

    float currentVolume = mic.getVolume();
    Serial.println("Published sensor data. Current volume: " + String(currentVolume));
}
