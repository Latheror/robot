#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "mqtt_handler.h"
#include "settings.h"
#include "servos.h"
#include "mbedtls/base64.h"
#include "Speaker.h"

WiFiClient espClient;
PubSubClient mqttClient(espClient);
extern Speaker speaker;

// --- Audio chunk storage ---
std::vector<String> audioChunks;
std::vector<size_t> chunkDecodedLengths; // taille décodée de chaque chunk
int expectedTotalChunks = 0;
size_t totalDecodedSize = 0;

// Forward declarations
void handleCommandMessage(const char* message);
void handleAudioMessage(const char* message);
void decodeAndPlayChunks();

/**
 * MQTT setup
 */
bool setupMQTT() {
    Serial.println("[MQTT] Initializing...");

    mqttClient.setBufferSize(50000);
    mqttClient.setServer(mqtt_broker, mqtt_port);

    mqttClient.setCallback([](char* topic, byte* payload, unsigned int length) {
        Serial.print("[MQTT] Message received on topic: ");
        Serial.println(topic);

        char message[length + 1];
        memcpy(message, payload, length);
        message[length] = '\0';

        Serial.print("[MQTT] Payload: ");
        Serial.println(message);

        if (strcmp(topic, "robot/1/commands") == 0) handleCommandMessage(message);
        if (strcmp(topic, "robot/1/audio") == 0) handleAudioMessage(message);
    });

    return reconnectMQTT();
}

/**
 * Command handling
 */
void handleCommandMessage(const char* message) {
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, message);
    if (error) {
        Serial.print("[MQTT] JSON parse error: ");
        Serial.println(error.f_str());
        return;
    }

    if (!doc.containsKey("servos")) return;

    JsonObject servos = doc["servos"];
    if (servos.containsKey("root"))     setTargetAngle(SERVO_ROOT,    servos["root"]);
    if (servos.containsKey("arm_a1"))   setTargetAngle(SERVO_ARM_A1,  servos["arm_a1"]);
    if (servos.containsKey("arm_b"))    setTargetAngle(SERVO_ARM_B,   servos["arm_b"]);
    if (servos.containsKey("wrist_a"))  setTargetAngle(SERVO_WRIST_A, servos["wrist_a"]);
    if (servos.containsKey("wrist_b"))  setTargetAngle(SERVO_WRIST_B, servos["wrist_b"]);
    if (servos.containsKey("gripper"))  setTargetAngle(SERVO_GRIPPER, servos["gripper"]);

    Serial.println("[MQTT] Updated servo target angles");
}

/**
 * Audio chunk handling
 */
void handleAudioMessage(const char* message) {
    Serial.println("[AUDIO] handleAudioMessage called");

    StaticJsonDocument<4096> doc;
    DeserializationError error = deserializeJson(doc, message);
    if (error) {
        Serial.print("[AUDIO] JSON parse error: ");
        Serial.println(error.f_str());
        return;
    }

    if (!doc.containsKey("message") || !doc.containsKey("chunk_index") || !doc.containsKey("total_chunks")) {
        Serial.println("[AUDIO] JSON missing required fields");
        return;
    }

    const char* base64Part = doc["message"];
    int chunkIndex = doc["chunk_index"];
    int totalChunks = doc["total_chunks"];

    Serial.printf("[AUDIO] Received chunk %d / %d, length %d\n", chunkIndex, totalChunks, (int)strlen(base64Part));

    // First chunk: initialize storage
    if (expectedTotalChunks != totalChunks) {
        audioChunks.clear();
        chunkDecodedLengths.clear();
        audioChunks.resize(totalChunks);
        chunkDecodedLengths.resize(totalChunks, 0);
        expectedTotalChunks = totalChunks;
        totalDecodedSize = 0;
        Serial.println("[AUDIO] Initialized audio chunk storage");
    }

    if (chunkIndex < 0 || chunkIndex >= totalChunks) {
        Serial.println("[AUDIO] Invalid chunk index");
        return;
    }

    audioChunks[chunkIndex] = base64Part;

    // Compute decoded size for this chunk
    size_t decodedLen = 0;
    if (mbedtls_base64_decode(nullptr, 0, &decodedLen, (const unsigned char*)base64Part, strlen(base64Part)) != MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL) {
        Serial.println("[AUDIO] Failed to get chunk decoded length");
        return;
    }
    chunkDecodedLengths[chunkIndex] = decodedLen;

    // Check if all chunks are received
    bool allReceived = true;
    for (int i = 0; i < totalChunks; i++) {
        if (audioChunks[i].length() == 0) {
            allReceived = false;
            break;
        }
    }

    if (allReceived) {
        // Sum total decoded size
        totalDecodedSize = 0;
        for (int i = 0; i < totalChunks; i++) totalDecodedSize += chunkDecodedLengths[i];

        Serial.printf("[AUDIO] All chunks received, total decoded size ~%d bytes\n", (int)totalDecodedSize);
        decodeAndPlayChunks();

        // Reset for next audio
        audioChunks.clear();
        chunkDecodedLengths.clear();
        expectedTotalChunks = 0;
        totalDecodedSize = 0;
        Serial.println("[AUDIO] Ready for next audio message");
    }
}

/**
 * Decode all chunks directly into a single buffer and play
 */
void decodeAndPlayChunks() {
    uint8_t* buffer = new uint8_t[totalDecodedSize];
    if (!buffer) {
        Serial.println("[AUDIO] Failed to allocate audio buffer");
        return;
    }

    size_t offset = 0;
    for (int i = 0; i < audioChunks.size(); i++) {
        size_t outLen = 0;
        int ret = mbedtls_base64_decode(buffer + offset, chunkDecodedLengths[i], &outLen,
                                        (const unsigned char*)audioChunks[i].c_str(),
                                        audioChunks[i].length());
        if (ret != 0) {
            Serial.printf("[AUDIO] Base64 decode failed for chunk %d\n", i);
            delete[] buffer;
            return;
        }
        offset += outLen;
    }

    Serial.printf("[AUDIO] Decoded %d bytes, playing...\n", (int)totalDecodedSize);
    speaker.playWavFromBuffer(buffer, totalDecodedSize);
    delete[] buffer;
    Serial.println("[AUDIO] Playback finished");
}

/**
 * MQTT reconnect
 */
bool reconnectMQTT() {
    int attempts = 0;
    while (!mqttClient.connected() && attempts < 3) {
        Serial.println("[MQTT] Attempting to connect...");
        if (mqttClient.connect(mqtt_client_id)) {
            Serial.println("[MQTT] Connected successfully");
            mqttClient.subscribe("robot/1/commands");
            mqttClient.subscribe("robot/1/audio");
            Serial.println("[MQTT] Ready to receive messages");
            return true;  // Successfully connected
        } else {
            attempts++;
            Serial.printf("[MQTT] Connection failed, rc=%d (Attempt %d/3)\n", mqttClient.state(), attempts);
        }
        delay(1000);
    }
    return false;  // Failed to connect after all attempts
}

void handleMQTT() {
    static unsigned long lastReconnectAttempt = 0;
    unsigned long now = millis();
    
    if (!mqttClient.connected()) {
        // Try to reconnect every 5 seconds
        if (now - lastReconnectAttempt > 5000) {
            lastReconnectAttempt = now;
            if (reconnectMQTT()) {
                lastReconnectAttempt = 0;
            }
        }
        return;  // Skip loop() if not connected
    }
    
    mqttClient.loop();
}

bool publishMessage(const char* topic, const char* message) {
    if (!mqttClient.connected()) return false;
    return mqttClient.publish(topic, message);
}
