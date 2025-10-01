#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <numeric>   // for std::accumulate
#include "mqtt_handler.h"
#include "settings.h"
#include "servos.h"
#include "mbedtls/base64.h"
#include "Speaker.h"

// MQTT client setup
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
extern Speaker speaker;

// Audio message handling
struct AudioMessage {
    std::vector<String> chunks;
    std::vector<size_t> decodedLengths;
    int expectedChunks = 0;
    size_t totalSize = 0;
};

static AudioMessage audioMsg;

// Message handlers
void handleCommand(const char* message) {
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, message);
    if (error) {
        Serial.println("[MQTT] Failed to parse command");
        return;
    }

    if (!doc.containsKey("servos")) return;

    JsonObject servos = doc["servos"];
    const struct { const char* name; int servo; } servoMap[] = {
        {"root", static_cast<int>(Joint::ROOT)}, 
        {"arm_a1", static_cast<int>(Joint::ARM_A1)},
        {"arm_b", static_cast<int>(Joint::ARM_B)},
        {"wrist_a", static_cast<int>(Joint::WRIST_A)},
        {"wrist_b", static_cast<int>(Joint::WRIST_B)},
        {"gripper", static_cast<int>(Joint::GRIPPER)}
    };

    for (const auto& map : servoMap) {
        if (servos.containsKey(map.name)) {
            ServoController::setTargetAngle(static_cast<Joint>(map.servo), servos[map.name]);
        }
    }
}

void handleAudio(const char* message) {
    StaticJsonDocument<4096> doc;
    if (deserializeJson(doc, message) != DeserializationError::Ok) {
        Serial.println("[MQTT] Failed to parse audio message");
        return;
    }

    // Validate required fields
    if (!doc.containsKey("message") || !doc.containsKey("chunk_index") || !doc.containsKey("total_chunks")) {
        return;
    }

    int chunkIndex = doc["chunk_index"];
    int totalChunks = doc["total_chunks"];
    const char* base64Data = doc["message"];

    // Initialize on first chunk
    if (audioMsg.expectedChunks != totalChunks) {
        audioMsg = AudioMessage();
        audioMsg.chunks.resize(totalChunks);
        audioMsg.decodedLengths.resize(totalChunks, 0);
        audioMsg.expectedChunks = totalChunks;
    }

    // Store chunk
    if (chunkIndex >= 0 && chunkIndex < totalChunks) {
        audioMsg.chunks[chunkIndex] = base64Data;
        size_t decodedLen = 0;
        mbedtls_base64_decode(nullptr, 0, &decodedLen, 
            (const unsigned char*)base64Data, strlen(base64Data));
        audioMsg.decodedLengths[chunkIndex] = decodedLen;
    }

    // Process complete message
    bool isComplete = std::none_of(audioMsg.chunks.begin(), audioMsg.chunks.end(),
                                 [](const String& s) { return s.isEmpty(); });
    if (isComplete) {
        // Calculate total size
        audioMsg.totalSize = std::accumulate(audioMsg.decodedLengths.begin(), 
                                           audioMsg.decodedLengths.end(), 0);

        // Allocate buffer and decode
        if (uint8_t* buffer = new uint8_t[audioMsg.totalSize]) {
            size_t offset = 0;
            for (int i = 0; i < totalChunks; i++) {
                size_t outLen = 0;
                mbedtls_base64_decode(buffer + offset, audioMsg.decodedLengths[i], &outLen,
                    (const unsigned char*)audioMsg.chunks[i].c_str(),
                    audioMsg.chunks[i].length());
                offset += outLen;
            }

            speaker.playBuffer(buffer, audioMsg.totalSize);
            delete[] buffer;
        }

        // Reset for next message
        audioMsg = AudioMessage();
    }
}

// Forward declarations
bool reconnectMQTT();

// MQTT core functions
bool setupMQTT() {
    mqttClient.setBufferSize(50000);
    mqttClient.setServer(NetworkConfig::MQTT_BROKER, NetworkConfig::MQTT_PORT);
    mqttClient.setCallback([](char* topic, byte* payload, unsigned int length) {
        char message[length + 1];
        memcpy(message, payload, length);
        message[length] = '\0';

        if (strcmp(topic, MQTT_TOPIC_COMMANDS) == 0) handleCommand(message);
        else if (strcmp(topic, MQTT_TOPIC_AUDIO) == 0) handleAudio(message);
    });

    return reconnectMQTT();
}

bool reconnectMQTT() {
    for (int attempt = 0; attempt < 3 && !mqttClient.connected(); attempt++) {
        if (mqttClient.connect(NetworkConfig::MQTT_CLIENT_ID)) {
            mqttClient.subscribe(MQTT_TOPIC_COMMANDS);
            mqttClient.subscribe(MQTT_TOPIC_AUDIO);
            return true;
        }
        delay(1000);
    }
    return false;
}

void handleMQTT() {
    static unsigned long lastReconnectAttempt = 0;
    if (!mqttClient.connected()) {
        if (millis() - lastReconnectAttempt > 5000) {
            lastReconnectAttempt = millis();
            if (reconnectMQTT()) lastReconnectAttempt = 0;
        }
        return;
    }
    mqttClient.loop();
}

bool publishMessage(const char* topic, const char* message) {
    return mqttClient.connected() && mqttClient.publish(topic, message);
}
