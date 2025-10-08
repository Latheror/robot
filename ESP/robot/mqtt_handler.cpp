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

MqttHandler::MqttHandler() {
    // mqttClient is already initialized internally with wifiClient
    audioState = {};
}

bool MqttHandler::setup() {
    Serial.println("[MQTT] Initializing...");

    mqttClient.setBufferSize(50000);
    mqttClient.setServer(NetworkConfig::MQTT_BROKER, NetworkConfig::MQTT_PORT);
    mqttClient.setCallback([this](char* topic, byte* payload, unsigned int length) {
        char message[length + 1];
        memcpy(message, payload, length);
        message[length] = '\0';

        if (strcmp(topic, MQTT_TOPIC_COMMANDS) == 0) handleCommand(message);
        else if (strcmp(topic, MQTT_TOPIC_AUDIO) == 0) handleAudio(message);
    });

    return reconnect();
}

bool MqttHandler::reconnect() {
    Serial.println("[MQTT] Attempting to connect...");

    for (int attempt = 0; attempt < 3 && !mqttClient.connected(); attempt++) {
        if (mqttClient.connect(NetworkConfig::MQTT_CLIENT_ID)) {
            mqttClient.subscribe(MQTT_TOPIC_COMMANDS);
            mqttClient.subscribe(MQTT_TOPIC_AUDIO);
            Serial.println("[MQTT] Connected and subscribed");
            return true;
        }
        delay(1000);
        Serial.println("[MQTT] Retry connecting...");
    }

    Serial.println("[MQTT] Failed to connect");
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

    indicators.set(Indicators::LED_PINS::MOTORS_MOVING, true);

    for (const auto& map : servoMap) {
        if (servos.containsKey(map.name)) {
            ServoController::setTargetAngle(static_cast<Joint>(map.servo), servos[map.name]);
        }
    }

    indicators.set(Indicators::LED_PINS::MOTORS_MOVING, false);

    if (onCommandReceivedCallback) onCommandReceivedCallback();
}

// --- Audio handling ---
void MqttHandler::handleAudio(const char* message) {
    StaticJsonDocument<4096> doc;
    if (deserializeJson(doc, message)) {
        Serial.println("[MQTT] Failed to parse audio message");
        return;
    }

    if (!doc.containsKey("message") || !doc.containsKey("chunk_index") || !doc.containsKey("total_chunks")) {
        Serial.println("[MQTT] Invalid audio message format");
        return;
    }

    int chunkIndex = doc["chunk_index"];
    int totalChunks = doc["total_chunks"];
    const char* base64Data = doc["message"];

    if (chunkIndex == 0) {
        if (LittleFS.exists(TEMP_AUDIO_FILE)) LittleFS.remove(TEMP_AUDIO_FILE);

        audioState.audioFile = LittleFS.open(TEMP_AUDIO_FILE, FILE_WRITE);
        if (!audioState.audioFile) {
            Serial.println("[MQTT] Failed to open temp audio file");
            return;
        }

        audioState.expectedChunks = totalChunks;
        Serial.println("[MQTT] Started writing audio file...");
    }

    if (!audioState.audioFile) {
        Serial.println("[MQTT] Audio file not open!");
        return;
    }

    size_t decodedLen = 0;
    mbedtls_base64_decode(nullptr, 0, &decodedLen, (const unsigned char*)base64Data, strlen(base64Data));

    uint8_t* buffer = new uint8_t[decodedLen];
    if (mbedtls_base64_decode(buffer, decodedLen, &decodedLen, (const unsigned char*)base64Data, strlen(base64Data)) != 0) {
        Serial.println("[MQTT] Base64 decode failed");
        delete[] buffer;
        return;
    }

    audioState.audioFile.write(buffer, decodedLen);
    delete[] buffer;

    if (chunkIndex == totalChunks - 1) {
        audioState.audioFile.close();
        Serial.println("[MQTT] Finished writing audio file, playing...");
        speaker.playWav(TEMP_AUDIO_FILE);
        audioState = {};  // reset
    } else {
        audioState.lastChunkIndex = chunkIndex;
    }
}
