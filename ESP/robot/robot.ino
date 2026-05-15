/**
 * @file robot.ino
 * @brief Main ESP32 robot firmware entry point and FreeRTOS task orchestration.
 */

// Standard libraries
#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <vector>

// Custom headers
#include "wifi_manager.h"
#include "mqtt_handler.h"
#include "oled_display.h"
#include "roboeyes_display.h"
#include "servos.h"
#include "speaker.h"
#include "indicators.h"
#include "INMP441.h"
#include "audio_recording.h"
#include "rgb_led.h"
#include "led_strip.h"
#include "i2c_scanner.h"

// --- Timing constants ---
const unsigned long mqttInterval = TaskConfig::SENSOR_SEND_INTERVAL_MS;

// --- Hardware modules ---
LEDStrip strip;
Indicators indicators(strip);
MqttHandler mqttHandler;
Speaker speaker;
INMP441 mic;
AudioRecording audioRecording(indicators, speaker);
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
bool micReady = false;
bool speakerReady = false;

// --- Functions ---
/**
 * @brief Create a pinned FreeRTOS task and log creation failures.
 */
bool createPinnedTask(TaskFunction_t taskFunction,
                      const char* taskName,
                      uint32_t stackDepth,
                      UBaseType_t priority,
                      TaskHandle_t* taskHandle,
                      BaseType_t coreId)
{
    BaseType_t result = xTaskCreatePinnedToCore(
        taskFunction,
        taskName,
        stackDepth,
        nullptr,
        priority,
        taskHandle,
        coreId);

    if (result != pdPASS) {
        Serial.printf("[TASK] Failed to create task '%s' (error=%ld)\n", taskName, static_cast<long>(result));
        return false;
    }

    Serial.printf("[TASK] Created task '%s' on core %ld\n", taskName, static_cast<long>(coreId));
    return true;
}

/**
 * @brief Sends mock sensor data via MQTT.
 * 
 * Generates random temperature, humidity, and light values and publishes them
 * as a JSON message to the MQTT sensor topic.
 */
void sendSensorData()
{
    unsigned long now = millis();
    char sensorMsg[512];

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

/**
 * @brief Task for updating the RoboEyes display.
 * 
 * Runs at ~100 FPS to animate the robot's eyes.
 */
void RoboEyesTask(void *pvParameters)
{
    const TickType_t delayTicks = pdMS_TO_TICKS(TaskConfig::ROBO_EYES_DELAY_MS); // ~100 FPS
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
        vTaskDelay(pdMS_TO_TICKS(TaskConfig::SERVO_UPDATE_DELAY_MS));
    }
}

void MicTask(void *pvParameters)
{
    const int NUM_SAMPLES = SystemConfig::MIC_NUM_SAMPLES;  // 256 samples
    int32_t buffer[NUM_SAMPLES];
    
    while (true)
    {
        mic.update();
        
        // Update audio recording with current volume for auto-stop detection
        audioRecording.update(mic.getVolume());
        
        // If recording, read samples and add to buffer
        if (audioRecording.isRecording()) {
            int samplesRead = mic.readSamplesForRecording(buffer, NUM_SAMPLES);
            if (samplesRead > 0) {
                std::vector<int32_t> samples(buffer, buffer + samplesRead);
                audioRecording.addSamples(samples);
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(TaskConfig::MIC_UPDATE_DELAY_MS));
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
    const unsigned long retryInterval = TaskConfig::WIFI_RETRY_INTERVAL_MS;
    const unsigned long attemptTimeout = NetworkConfig::WIFI_TIMEOUT_MS;
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
            if (attempting && (now - lastAttempt > attemptTimeout))
            {
                Serial.println("WiFi connection attempt timed out; scheduling retry...");
                WiFi.disconnect(false, false);
                attempting = false;
            }

            if (!attempting && (lastAttempt == 0 || now - lastAttempt > retryInterval))
            {
                Serial.println("Attempting WiFi reconnect...");
                WiFi.begin(NetworkConfig::WIFI_SSID, NetworkConfig::WIFI_PASSWORD);
                attempting = true;
                lastAttempt = now;
            }
        }

        //Serial.printf("WiFi status: %d\n", status);
        vTaskDelay(pdMS_TO_TICKS(TaskConfig::WIFI_STATUS_CHECK_INTERVAL_MS));
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
                vTaskDelay(pdMS_TO_TICKS(2000)); // Delay to allow WiFi connection sound to finish
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

        vTaskDelay(pdMS_TO_TICKS(TaskConfig::MQTT_HANDLE_DELAY_MS));
    }
}

// --- Sensor Task ---
void SensorTask(void *pvParameters)
{
    while (true)
    {
        if (!wifiConnected || !mqttHandler.isConnected())
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
        vTaskDelay(pdMS_TO_TICKS(TaskConfig::HEAP_CHECK_INTERVAL_MS));
    }
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

    // Set I2C timeout to prevent blocking (5ms)
    Wire.setTimeout(3000);
    Wire.begin(PinConfig::I2C_SDA, PinConfig::I2C_SCL);  // Initialize I2C bus with correct pins

    // --- Initialize Serial FIRST (synchronously) ---
    Serial.begin(SystemConfig::SERIAL_BAUD_RATE);
    unsigned long serialStart = millis();
    while (!Serial && millis() - serialStart < TaskConfig::SERIAL_INIT_TIMEOUT_MS)
    {
        delay(SystemConfig::SERIAL_INIT_DELAY_MS);
    }
    Serial.println("Serial initialized.");

    // --- Scan I2C bus for connected devices ---
    i2cScan();

    // --- Initialize hardware SYNCHRONOUSLY before FreeRTOS tasks ---
    strip.begin();
    indicators.begin();
    if (!oled.begin()) {
        Serial.println("[OLED] Initialization failed - display disabled");
    }
    if (!ServoController::begin()) {
        indicators.setColor(Indicators::LED_PINS::MOTORS_MOVING, 255, 0, 0); // Red if not initialized
    } else {
        indicators.setColor(Indicators::LED_PINS::MOTORS_MOVING, 0, 255, 0); // Green if initialized
    }
    roboEyes.begin();
    speakerReady = speaker.begin();
    if (!speakerReady) {
        Serial.println("[SPEAKER] Initialization failed");
    } else {
        speaker.setPlaybackCallback([](bool isPlaying) {
            indicators.setColor(Indicators::LED_PINS::IS_SPEAKING, 0, 0, isPlaying ? 255 : 0);
            if (isPlaying) {
                mic.disableClapDetection();
            } else if (!audioRecording.isRecording()) {
                mic.enableClapDetection();
            }
        });
    }
    rgbLed.begin();
    rgbLed.setBrightness(0);
    rgbLed.clear();

    // --- Microphone setup ---
    micReady = mic.begin();
    if (micReady)
    {
        Serial.println("Microphone ready.");
        mic.setClapCallback([]()
                            {
                                if (audioRecording.isRecording()) {
                                    Serial.println("Recording already in progress, ignoring clap trigger");
                                    return;
                                }
                                Serial.println("Double clap detected - starting audio recording!");
                                if (speakerReady) {
                                    speaker.playWav("/yesilisten.wav");
                                }
                                audioRecording.startRecording();
                                // Disable clap detection during recording to prevent noise from restarting it
                                mic.disableClapDetection();
                            });
        
        // Set up audio recording callback
        audioRecording.setRecordingFinishedCallback([](const char* filePath) {
            Serial.printf("Audio recording finished: %s\n", filePath);
            // Re-enable clap detection after recording finishes
            mic.enableClapDetection();
        });
    }
    else
    {
        Serial.println("[MIC] Initialization failed.");
    }

    if (speakerReady) {
        speaker.listFiles();
        speaker.playWav("/start_speech.wav");
    }

    // --- Create FreeRTOS tasks AFTER all hardware is initialized ---
    // Core 0: Network and background tasks
    createPinnedTask(WiFiTask, "WiFi", 4096, 2, NULL, 0);
    createPinnedTask(MqttTask, "MQTT", 4096, 1, &mqttTaskHandle, 0);
    createPinnedTask(SensorTask, "Sensor", 4096, 1, &sensorTaskHandle, 0);
    createPinnedTask(CheckHeapTask, "CheckHeap", 4096, 1, &checkHeapTaskHandle, 0);

    // Core 1: Real-time tasks (audio, display, servos)
    createPinnedTask(RoboEyesTask, "RoboEyes", 4096, 2, &roboEyesTaskHandle, 1);
    if (ServoController::isInitialized()) {
        createPinnedTask(ServoTask, "Servo", 4096, 3, &servoTaskHandle, 1);
    }
    if (micReady) {
        createPinnedTask(MicTask, "Mic", 10240, 4, &micTaskHandle, 1);
    }

    // Configure Robot Arm LED based on initialization status
    if (!ServoController::isInitialized()) {
        indicators.setColor(Indicators::LED_PINS::MOTORS_MOVING, 255, 0, 0); // Red if not initialized
    } else {
        indicators.setColor(Indicators::LED_PINS::MOTORS_MOVING, 0, 255, 0); // Green if initialized
    }
}

// --- Loop ---
void loop()
{
    vTaskDelay(portMAX_DELAY); // All work handled by FreeRTOS
}
