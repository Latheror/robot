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
#include "led_strip.h"

// --- Timing constants ---
const unsigned long mqttInterval = 120 * 1000;

// --- Hardware modules ---
LEDStrip strip;
Indicators indicators(strip);
MqttHandler mqttHandler;
Speaker speaker;
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
TaskHandle_t ledTaskHandle;

// --- Global WiFi state flag ---
volatile bool wifiConnected = false;

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

    mqttHandler.publishMessage(MqttHandler::MQTT_TOPIC_SENSORS, sensorMsg);
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

// --- WiFi Task ---
void WiFiTask(void *pvParameters)
{
    WiFi.mode(WIFI_STA);
    Serial.println("Connecting to WiFi...");
    
    // Show red LED while disconnected
    indicators.setColor(Indicators::LED_PINS::WIFI, 255, 0, 0);

    unsigned long lastAttempt = 0;
    const unsigned long retryInterval = 10000;
    bool attempting = false;

    while (true)
    {
        wl_status_t status = WiFi.status();

        if (status == WL_CONNECTED)
        {
            if (!wifiConnected)
            {
                wifiConnected = true;
                Serial.println("WiFi connected!");
                Serial.print("IP: ");
                Serial.println(WiFi.localIP());

                indicators.blink(Indicators::LED_PINS::WIFI, 3, 200);
                indicators.set(Indicators::LED_PINS::WIFI, true);
                speaker.playWav("/connected_to_wifi.wav");
            }
            attempting = false;
        }
        else
        {
            if (wifiConnected)
            {
                wifiConnected = false;
                Serial.println("WiFi lost!");
                indicators.set(Indicators::LED_PINS::WIFI, false);
                WiFi.disconnect(true, false);
                attempting = false;
            }

            unsigned long now = millis();
            if (!attempting && (now - lastAttempt > retryInterval))
            {
                Serial.println("Attempting WiFi reconnect...");
                WiFi.begin(NetworkConfig::WIFI_SSID, NetworkConfig::WIFI_PASSWORD);
                attempting = true;
                lastAttempt = now;
            }
        }

        //Serial.printf("WiFi status: %d\n", status);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// --- MQTT Task ---
void MqttTask(void *pvParameters)
{
    bool mqttConnected = false;

    while (true)
    {
        if (!wifiConnected)
        {
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }

        // Ensure MQTT is connected
        if (!mqttConnected)
        {
            Serial.println("Connecting to MQTT broker...");
            if (mqttHandler.setup())
            {
                mqttConnected = true;
                Serial.println("MQTT connected.");

                mqttHandler.setOnCommandReceivedCallback([]()
                {
                    Serial.println("Command executed callback triggered.");
                    indicators.blink(Indicators::LED_PINS::MOTORS_MOVING, 3, 100);
                });

                indicators.set(Indicators::LED_PINS::MQTT, true);
                speaker.playWav("/connected_to_server.wav");
            }
            else
            {
                Serial.println("MQTT connection failed. Retrying...");
                vTaskDelay(pdMS_TO_TICKS(5000));
                continue;
            }
        }

        // Handle MQTT messages
        mqttHandler.handle();

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// --- Sensor Task ---
void SensorTask(void *pvParameters)
{
    while (true)
    {
        if (!wifiConnected)
        {
            vTaskDelay(pdMS_TO_TICKS(2000)); // Wait for WiFi
            continue;
        }

        sendSensorData();
        vTaskDelay(pdMS_TO_TICKS(mqttInterval));
    }
}

// --- Heap Monitoring Task ---
void CheckHeapTask(void *pvParameters)
{
    while (true)
    {
        Serial.printf("[HEAP] Free heap: %u bytes\n", esp_get_free_heap_size());
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// --- Serial Initialization Task ---
void SerialInitTask(void *pvParameters)
{
    Serial.begin(SystemConfig::SERIAL_BAUD_RATE);
    unsigned long serialStart = millis();
    while (!Serial && millis() - serialStart < 2000)
    {
        vTaskDelay(pdMS_TO_TICKS(10)); // non-blocking delay
    }
    Serial.println("Serial initialized.");

    vTaskDelete(NULL); // kill this task once done
}

// All LED status handling is now done through the Indicators class in their respective tasks:
// - WiFiTask: LED 1 - Red when disconnected, Green when connected
// - MqttTask: LED 2 - Red when connecting, Green when connected, Off when disconnected
// - INMP441: LED 3 - On when listening
// - Speaker: LED 4 - Blinks when speaking
// - Servos:  LED 5 - Purple when moving

// --- Setup ---
void setup()
{
    esp_log_level_set("i2c.master", ESP_LOG_NONE);

    // --- Initialize Serial ---
    xTaskCreate(SerialInitTask, "SerialInit", 1024, NULL, 1, NULL);

    // --- Initialize hardware ---
    indicators.begin();
    oled.begin();
    ServoController::begin();
    roboEyes.begin();
    speaker.begin();
    speaker.setPlaybackCallback([](bool isPlaying) {
        // Blue when speaking, off when silent
        indicators.setColor(Indicators::LED_PINS::IS_SPEAKING, isPlaying ? 0 : 0, isPlaying ? 0 : 0, isPlaying ? 255 : 0);
    });
    rgbLed.begin();
    rgbLed.setBrightness(50);
    rgbLed.setColor(0, 0, 255);
    strip.begin();

    speaker.listFiles();
    speaker.playWav("/start_speech.wav");

    // --- Microphone setup ---
    if (mic.begin())
    {
        Serial.println("Microphone ready.");
        mic.setClapCallback([]()
                            {
                                Serial.println("Clap detected!");
                                speaker.playWav("/yesilisten.wav");
                            });
        mic.setIsRecordingCallback([](bool recording)
                                   {
                                       Serial.print("Recording state: ");
                                       Serial.println(recording ? "START" : "STOP");
                                       indicators.set(Indicators::LED_PINS::IS_LISTENING, recording);
                                   });
    }
    else
    {
        Serial.println("Microphone initialization failed.");
    }

    // --- Create FreeRTOS tasks ---
    xTaskCreatePinnedToCore(WiFiTask, "WiFi", 4096, NULL, 3, NULL, 0);
    xTaskCreatePinnedToCore(MqttTask, "MQTT", 4096, NULL, 2, &mqttTaskHandle, 1);
    xTaskCreatePinnedToCore(SensorTask, "Sensor", 4096, NULL, 2, &sensorTaskHandle, 1);
    xTaskCreate(RoboEyesTask, "RoboEyes", 4096, NULL, 3, &roboEyesTaskHandle);
    xTaskCreate(ServoTask, "Servo", 2048, NULL, 3, &servoTaskHandle);
    xTaskCreate(MicTask, "Mic", 4096, NULL, 2, &micTaskHandle);
    xTaskCreate(CheckHeapTask, "CheckHeap", 4096, NULL, 4, &checkHeapTaskHandle);
}

// --- Loop ---
void loop()
{
    vTaskDelay(portMAX_DELAY); // All work handled by FreeRTOS
}
