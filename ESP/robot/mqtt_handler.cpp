#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <numeric>   // for std::accumulate
#include <LittleFS.h>
#include "mqtt_handler.h"
#include "settings.h"
#include "servos.h"
#include "mbedtls/base64.h"
#include "Speaker.h"

// MQTT client setup
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
extern Speaker speaker;

// Temporary WAV file
static const char* TEMP_AUDIO_FILE = "/temp_audio.wav";

// Audio message state
struct AudioState {
    int expectedChunks = 0;
    int lastChunkIndex = -1;
    File audioFile;
};

static AudioState audioState;

// --- Command handling ---
void handleCommand(const char* message) {
    Serial.println("[MQTT] Received command");

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

// --- Audio handling ---
void handleAudio(const char* message) {
    StaticJsonDocument<4096> doc;
    if (deserializeJson(doc, message) != DeserializationError::Ok) {
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

    // Initialize on first chunk
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

    // Decode the base64 chunk
    size_t decodedLen = 0;
    // Only get the decoded length, ignore the return code
    mbedtls_base64_decode(nullptr, 0, &decodedLen, (const unsigned char*)base64Data, strlen(base64Data));

    uint8_t* buffer = new uint8_t[decodedLen];
    if (mbedtls_base64_decode(buffer, decodedLen, &decodedLen, (const unsigned char*)base64Data, strlen(base64Data)) != 0) {
        Serial.println("[MQTT] Base64 decode failed");
        delete[] buffer;
        return;
    }

    // Write decoded chunk to file
    audioState.audioFile.write(buffer, decodedLen);
    delete[] buffer;

    // Check if this is the last chunk
    if (chunkIndex == totalChunks - 1) {
        audioState.audioFile.close();
        Serial.println("[MQTT] Finished writing audio file, playing...");
        speaker.playWav(TEMP_AUDIO_FILE);  // Call your speaker
        audioState.expectedChunks = 0;
        audioState.lastChunkIndex = -1;
    } else {
        audioState.lastChunkIndex = chunkIndex;
    }
}


// --- MQTT setup ---
bool reconnectMQTT();
bool setupMQTT() {
    Serial.println("[MQTT] Initializing...");

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
    Serial.println("[MQTT] Attempting to connect...");

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

// --- MQTT publishing ---
bool publishMessage(const char* topic, const char* message) {
    Serial.printf("[MQTT] Publishing to %s: %s\n", topic, message);
    return mqttClient.connected() && mqttClient.publish(topic, message);
}
