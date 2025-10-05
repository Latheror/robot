#include <Wire.h>
#include <WiFi.h>
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
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// --- Timing constants ---
const unsigned long mqttInterval = 120 * 1000;

// --- Hardware modules ---
Speaker speaker;
Indicators indicators;
INMP441 mic;
RGBLed rgbLed;
OLEDDisplay oled;
RoboEyesDisplay roboEyes(oled);

// --- FreeRTOS task handles ---
TaskHandle_t roboEyesTaskHandle;
TaskHandle_t servoTaskHandle;
TaskHandle_t micTaskHandle;
TaskHandle_t mqttTaskHandle;
TaskHandle_t sensorTaskHandle;
TaskHandle_t checkHeapTaskHandle;

// --- Functions ---
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

    indicators.blink(Indicators::LED::ACTIVITY, 3, 200);
}

// --- FreeRTOS Tasks ---
void RoboEyesTask(void *pvParameters)
{
    const TickType_t delayTicks = pdMS_TO_TICKS(10); // ~100 FPS
    while (true)
    {
        roboEyes.update();
        vTaskDelay(delayTicks);
    }
}

void ServoTask(void *pvParameters)
{
    while (true)
    {
        ServoController::update();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void MicTask(void *pvParameters)
{
    while (true)
    {
        mic.update();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void MqttTask(void *pvParameters)
{
    while (true)
    {
        handleMQTT();

        // WiFi reconnect if disconnected
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("WiFi disconnected. Attempting to reconnect...");
            WiFi.reconnect();
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void SensorTask(void *pvParameters)
{
    while (true)
    {
        sendSensorData();
        vTaskDelay(pdMS_TO_TICKS(mqttInterval));
    }
}

void CheckHeapTask(void *pvParameters)
{
    while (true)
    {
        Serial.printf("[HEAP] Free heap: %u bytes\n", esp_get_free_heap_size());
        vTaskDelay(pdMS_TO_TICKS(5000)); // Check every 5 seconds
    }
}

// --- Setup ---
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
    rgbLed.setBrightness(50);
    rgbLed.setColor(0, 0, 255);

    // Play startup sound
    speaker.listFiles();
    speaker.playWav("/start_speech.wav");

    // Connect WiFi
    WiFi.mode(WIFI_STA);
    Serial.print("Connecting to WiFi");
    WiFi.begin(NetworkConfig::WIFI_SSID, NetworkConfig::WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    indicators.blink(Indicators::LED::STATUS, 3, 200);
    indicators.set(Indicators::LED::STATUS, true);
    speaker.playWav("/connected_to_wifi.wav");

    delay(500);

    // Setup MQTT
    if (setupMQTT())
    {
        Serial.println("MQTT connected.");
        speaker.playWav("/connected_to_server.wav");
        indicators.blink(Indicators::LED::NETWORK, 3, 200);
        indicators.set(Indicators::LED::NETWORK, true);
    }
    else
    {
        Serial.println("MQTT connection failed.");
    }

    // Initialize microphone
    if (mic.begin())
    {
        Serial.println("Microphone ready.");

        mic.setClapCallback([]()
                            {
            Serial.println("Clap detected!");
            speaker.playWav("/yesilisten.wav");
        });
    }
    else
    {
        Serial.println("Microphone initialization failed.");
    }

    // --- Create FreeRTOS tasks ---
    xTaskCreate(RoboEyesTask, "RoboEyes", 4096, NULL, 2, &roboEyesTaskHandle);
    xTaskCreate(ServoTask, "Servo", 2048, NULL, 2, &servoTaskHandle);
    xTaskCreate(MicTask, "Mic", 4096, NULL, 2, &micTaskHandle);
    xTaskCreate(MqttTask, "MQTT", 4096, NULL, 1, &mqttTaskHandle);
    xTaskCreate(SensorTask, "Sensor", 4096, NULL, 1, &sensorTaskHandle);
    xTaskCreate(CheckHeapTask, "CheckHeap", 4096, NULL, 1, &checkHeapTaskHandle);
}

// --- Loop ---
void loop()
{
    // FreeRTOS handles everything; just yield
    vTaskDelay(portMAX_DELAY);
}
