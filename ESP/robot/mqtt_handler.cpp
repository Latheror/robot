#include "mqtt_handler.h"
#include "settings.h"
#include "servos.h"
#include "Speaker.h"
#include "indicators.h"
#include <LittleFS.h>
#include <mbedtls/base64.h>
#include <Arduino.h>

extern Speaker speaker;
extern Indicators indicators;
extern SemaphoreHandle_t audioMutex;

MqttHandler::MqttHandler() {
    // mqttClient is already initialized internally with wifiClient
    audioState = {};
}

bool MqttHandler::setup() {
    Serial.println("[MQTT] Initializing...");

    mqttClient.setBufferSize(50000);
    mqttClient.setServer(NetworkConfig::MQTT_BROKER, NetworkConfig::MQTT_PORT);
    mqttClient.setCallback([this](char* topic, byte* payload, unsigned int length) {
        // Safety check: reject too-large messages
        if (length > 16384) {
            Serial.printf("[MQTT] Message too large (%u bytes), rejected\n", length);
            return;
        }
        
        // Allocate message buffer dynamically to avoid stack overflow
        std::unique_ptr<char[]> message(new char[length + 1]);
        if (!message) {
            Serial.println("[MQTT] Failed to allocate message buffer");
            return;
        }
        
        memcpy(message.get(), payload, length);
        message.get()[length] = '\0';
        
        if (strcmp(topic, MQTT_TOPIC_COMMANDS) == 0) {
            handleCommand(message.get());
        } else if (strcmp(topic, MQTT_TOPIC_AUDIO) == 0) {
            handleAudio(message.get());
        }
    });

    return reconnect();
}

bool MqttHandler::reconnect() {
    Serial.println("[MQTT] Attempting to connect...");

    // Red for connecting
    indicators.setColor(Indicators::LED_PINS::MQTT, 255, 0, 0);

    for (int attempt = 0; attempt < 3 && !mqttClient.connected(); attempt++) {
        if (mqttClient.connect(NetworkConfig::MQTT_CLIENT_ID)) {
            mqttClient.subscribe(MQTT_TOPIC_COMMANDS);
            mqttClient.subscribe(MQTT_TOPIC_AUDIO);
            Serial.println("[MQTT] Connected and subscribed");
            // Green for connected
            indicators.set(Indicators::LED_PINS::MQTT, true);
            return true;
        }
        delay(1000);
        Serial.println("[MQTT] Retry connecting...");
    }

    Serial.println("[MQTT] Failed to connect");
    // Turn LED off for disconnected state
    indicators.set(Indicators::LED_PINS::MQTT, false);
    return false;
}

void MqttHandler::handle() {
    static unsigned long lastReconnectAttempt = 0;

    if (!mqttClient.connected()) {
        if (millis() - lastReconnectAttempt > 5000) {
            lastReconnectAttempt = millis();
            reconnect();
        }
        return;
    }

    mqttClient.loop();
}

bool MqttHandler::publishMessage(const char* topic, const char* message) {
    Serial.printf("[MQTT] Publishing to %s: %s\n", topic, message);
    return mqttClient.connected() && mqttClient.publish(topic, message);
}

void MqttHandler::setOnCommandReceivedCallback(std::function<void()> callback) {
    onCommandReceivedCallback = callback;
}

// --- Command handling ---
void MqttHandler::handleCommand(const char* message) {
    // Validate input
    if (!message) {
        Serial.println("[MQTT] Error: NULL message pointer in handleCommand");
        return;
    }
    
    static int lastRandNum = 0;

    switch (lastRandNum) {
        case 0: speaker.playWav("/warning.wav"); break;
        case 1: speaker.playWav("/imabitclumsy.wav"); break;
        case 2: speaker.playWav("/iwilltrysomething.wav"); break;
    }
    lastRandNum = (lastRandNum + 1) % 3;

    Serial.println("[MQTT] Received command:");
    Serial.println(message);

    StaticJsonDocument<512> doc;
    if (deserializeJson(doc, message)) {
        Serial.println("[MQTT] Failed to parse command JSON");
        return;
    }

    if (!doc.containsKey("servos")) return;
    JsonObject servos = doc["servos"];

    const struct { const char* name; int servo; } servoMap[] = {
        {"root", static_cast<int>(Joint::ROOT)}, 
        {"arm_a", static_cast<int>(Joint::ARM_A)},
        {"arm_b", static_cast<int>(Joint::ARM_B)},
        {"wrist_a", static_cast<int>(Joint::WRIST_A)},
        {"wrist_b", static_cast<int>(Joint::WRIST_B)},
        {"gripper", static_cast<int>(Joint::GRIPPER)}
    };

    // Set motors indicator - Purple for movement
    indicators.setColor(Indicators::LED_PINS::MOTORS_MOVING, 255, 0, 255);

    if (!ServoController::isInitialized()) {
        Serial.println("[MQTT] Servo controller not initialized, ignoring command");
        indicators.set(Indicators::LED_PINS::MOTORS_MOVING, false);
        return;
    }

    for (const auto& map : servoMap) {
        if (servos.containsKey(map.name)) {
            JsonVariant angleVar = servos[map.name];
            if (!angleVar.is<float>() && !angleVar.is<int>()) {
                Serial.printf("[MQTT] Invalid angle type for joint '%s'\n", map.name);
                continue;
            }
            ServoController::setTargetAngle(static_cast<Joint>(map.servo), angleVar.as<float>());
        }
    }

    // Turn off movement LED
    indicators.set(Indicators::LED_PINS::MOTORS_MOVING, false);

    if (onCommandReceivedCallback) onCommandReceivedCallback();
}

// --- Audio handling ---
void MqttHandler::handleAudio(const char* message) {
    // Protect audioState with mutex to prevent race conditions
    if (!audioMutex) {
        Serial.println("[MQTT] Audio mutex not initialized, rejecting audio chunk");
        return;
    }
    
    if (xSemaphoreTake(audioMutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        Serial.println("[MQTT] Failed to acquire audio mutex (timeout), audio chunk rejected");
        return;
    }
    
    // Use DynamicJsonDocument for large payloads to avoid stack overflow
    // StaticJsonDocument allocates on stack, which causes overflow with large chunks
    DynamicJsonDocument doc(16384);
    if (deserializeJson(doc, message)) {
        Serial.println("[MQTT] Failed to parse audio message");
        xSemaphoreGive(audioMutex);
        return;
    }

    if (!doc.containsKey("message") || !doc.containsKey("chunk_index") || !doc.containsKey("total_chunks")) {
        Serial.println("[MQTT] Invalid audio message format");
        xSemaphoreGive(audioMutex);
        return;
    }

    int chunkIndex = doc["chunk_index"];
    int totalChunks = doc["total_chunks"];
    const char* base64Data = doc["message"];

    Serial.printf("[MQTT] Processing chunk %d/%d, base64 length: %u\n", chunkIndex, totalChunks, strlen(base64Data));

    if (chunkIndex == 0) {
        // Reset previous state if exists
        if (audioState.audioFile) {
            audioState.audioFile.close();
        }
        
        if (LittleFS.exists(TEMP_AUDIO_FILE)) LittleFS.remove(TEMP_AUDIO_FILE);

        audioState.audioFile = LittleFS.open(TEMP_AUDIO_FILE, FILE_WRITE);
        if (!audioState.audioFile) {
            Serial.println("[MQTT] Failed to open temp audio file");
            audioState = {};  // Reset state
            xSemaphoreGive(audioMutex);
            return;
        }

        audioState.expectedChunks = totalChunks;
        audioState.lastChunkIndex = -1;
        Serial.println("[MQTT] Started writing audio file...");
    }

    if (!audioState.audioFile) {
        Serial.println("[MQTT] Audio file not open! Aborting audio transfer");
        audioState = {};  // Reset state
        xSemaphoreGive(audioMutex);
        return;
    }
    
    // Validate chunk index sequence
    if (chunkIndex != audioState.lastChunkIndex + 1) {
        Serial.printf("[MQTT] Out-of-order chunk: expected %d, got %d. Resetting audio state.\n", 
                      audioState.lastChunkIndex + 1, chunkIndex);
        audioState.audioFile.close();
        audioState = {};  // Reset state
        xSemaphoreGive(audioMutex);
        return;
    }

    size_t decodedLen = 0;
    mbedtls_base64_decode(nullptr, 0, &decodedLen, (const unsigned char*)base64Data, strlen(base64Data));

    // Check heap before allocation - reserve at least 50% free
    uint32_t freeHeap = ESP.getFreeHeap();
    if (decodedLen > (freeHeap / 2)) {
        Serial.printf("[MQTT] Insufficient heap for decoding! Need: %zu, Free: %u\n", decodedLen, freeHeap);
        xSemaphoreGive(audioMutex);
        return;
    }

    std::unique_ptr<uint8_t[]> buffer(new uint8_t[decodedLen]);
    if (!buffer) {
        Serial.println("[MQTT] Failed to allocate buffer for decoding");
        xSemaphoreGive(audioMutex);
        return;
    }

    if (mbedtls_base64_decode(buffer.get(), decodedLen, &decodedLen, (const unsigned char*)base64Data, strlen(base64Data)) != 0) {
        Serial.println("[MQTT] Base64 decode failed");
        audioState.audioFile.close();
        audioState = {};  // Reset audio state
        xSemaphoreGive(audioMutex);
        return;
    }

    audioState.audioFile.write(buffer.get(), decodedLen);

    if (chunkIndex == totalChunks - 1) {
        audioState.audioFile.close();
        Serial.println("[MQTT] Finished writing audio file, playing...");
        xSemaphoreGive(audioMutex);  // Release before playback
        speaker.playWav(TEMP_AUDIO_FILE);
        xSemaphoreTake(audioMutex, pdMS_TO_TICKS(100));  // Re-acquire to reset state safely
        audioState = {};  // reset
        xSemaphoreGive(audioMutex);
    } else {
        audioState.lastChunkIndex = chunkIndex;
        xSemaphoreGive(audioMutex);  // Release mutex
    }
}
