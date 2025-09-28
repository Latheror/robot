#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "mqtt_handler.h"
#include "settings.h"
#include "servos.h"   // For setTargetAngle()

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Forward declaration
/**
 * @brief Handles incoming JSON command messages for servos.
 * 
 * This function parses a JSON string and updates the target angles
 * of the robot's servos if a "servos" object is present.
 * 
 * @param message Null-terminated JSON string received from MQTT.
 */
void handleCommandMessage(const char* message);

/**
 * @brief Handles incoming JSON audio messages and plays the audio.
 * 
 * This function parses a JSON string containing Base64-encoded audio data,
 * decodes it, and plays it using the AudioPlayer.
 * 
 * @param message Null-terminated JSON string received from MQTT.
 */
void handleAudioMessage(const char* message);

/**
 * @brief Initializes the MQTT client and sets up the callback for messages.
 * 
 * Configures the MQTT broker, topic subscriptions, and the message callback.
 * Automatically calls reconnectMQTT() to establish a connection if needed.
 */
void setupMQTT() {
    Serial.println("[MQTT] Initializing...");
    mqttClient.setServer(mqtt_broker, mqtt_port);

    // Set the callback function for receiving messages
    mqttClient.setCallback([](char* topic, byte* payload, unsigned int length) {
        Serial.print("[MQTT] Message received on topic: ");
        Serial.println(topic);

        // Convert payload to string
        char message[length + 1];
        memcpy(message, payload, length);
        message[length] = '\0';

        Serial.print("[MQTT] Message: ");
        Serial.println(message);

        // Handle commands topic
        if (strcmp(topic, "robot/1/commands") == 0) {
            Serial.println("[MQTT] Received command message");
            handleCommandMessage(message);
        }

        // Handle audio topic
        if (strcmp(topic, "robot/1/audio") == 0) {
            Serial.println("[MQTT] Received audio message");
            handleAudioMessage(message);
        }
    });

    // Initial connection attempt
    reconnectMQTT();
}

/**
 * @brief Parses a JSON command message and updates servo targets.
 * 
 * Expected JSON format:
 * {
 *   "servos": {
 *      "root": 90,
 *      "arm_a1": 120,
 *      "arm_b": 100,
 *      "wrist_a": 80,
 *      "wrist_b": 100,
 *      "gripper": 50
 *   }
 * }
 * 
 * @param message Null-terminated JSON string.
 */
void handleCommandMessage(const char* message) {
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, message);

    if (error) {
        Serial.print("[MQTT] JSON parse error: ");
        Serial.println(error.f_str());
        return;
    }

    if (!doc.containsKey("servos")) {
        Serial.println("[MQTT] No 'servos' object in JSON");
        return;
    }

    JsonObject servos = doc["servos"];

    if (servos.containsKey("root"))     setTargetAngle(SERVO_ROOT,    servos["root"]);
    if (servos.containsKey("arm_a1"))   setTargetAngle(SERVO_ARM_A1,  servos["arm_a1"]);
    if (servos.containsKey("arm_b"))    setTargetAngle(SERVO_ARM_B,   servos["arm_b"]);
    if (servos.containsKey("wrist_a"))  setTargetAngle(SERVO_WRIST_A, servos["wrist_a"]);
    if (servos.containsKey("wrist_b"))  setTargetAngle(SERVO_WRIST_B, servos["wrist_b"]);
    if (servos.containsKey("gripper"))  setTargetAngle(SERVO_GRIPPER, servos["gripper"]);

    Serial.println("[MQTT] Updated target angles from JSON command");
}

/**
 * @brief Handles incoming audio messages in Base64 and plays them.
 * 
 * Expected JSON format:
 * {
 *   "topic": "robot/1/audio",
 *   "message": "<base64-encoded wav data>"
 * }
 * 
 * @param message Null-terminated JSON string
 */
void handleAudioMessage(const char* message) {
    StaticJsonDocument<2048 * 10> doc; // adjust size depending on audio length
    DeserializationError error = deserializeJson(doc, message);

    if (error) {
        Serial.print("[MQTT] JSON parse error (audio): ");
        Serial.println(error.f_str());
        return;
    }

    if (!doc.containsKey("message")) {
        Serial.println("[MQTT] No 'message' field in audio JSON");
        return;
    }

    const char* base64Audio = doc["message"];
    size_t decodedLength = Base64.decodedLength(base64Audio);
    uint8_t* audioBuffer = new uint8_t[decodedLength];

    Base64.decode(audioBuffer, base64Audio, strlen(base64Audio));

    // Play audio via I2S/DAC
    AudioPlayer.playWAV(audioBuffer, decodedLength);

    delete[] audioBuffer;

    Serial.println("[MQTT] Audio played successfully");
}

/**
 * @brief Reconnects to the MQTT broker if the client is disconnected.
 * 
 * Attempts up to 3 times to reconnect. If successful, subscribes
 * to the "robot/1/commands" topic.
 */
void reconnectMQTT() {
    int attempts = 0;
    while (!mqttClient.connected() && attempts < 3) {
        Serial.println("[MQTT] Attempting to connect...");

        if (mqttClient.connect(mqtt_client_id)) {
            Serial.println("[MQTT] Connected successfully");

            // Subscribe to the command topic
            if (mqttClient.subscribe("robot/1/commands")) {
                Serial.println("[MQTT] Subscribed to robot/1/commands");
            } else {
                Serial.println("[MQTT] Failed to subscribe to robot/1/commands");
            }

            // Subscribe to the audio topic
            if (mqttClient.subscribe("robot/1/audio")) {
                Serial.println("[MQTT] Subscribed to robot/1/audio");
            } else {
                Serial.println("[MQTT] Failed to subscribe to robot/1/audio");
            }

            Serial.println("[MQTT] Ready to publish messages");
        } else {
            attempts++;
            Serial.print("[MQTT] Connection failed, rc=");
            Serial.print(mqttClient.state());
            Serial.printf(" (Attempt %d/3)\n", attempts);
            delay(1000);
        }
    }
}

/**
 * @brief Handles MQTT client loop and reconnects if necessary.
 * 
 * Should be called regularly in the main loop.
 */
void handleMQTT() {
    if (!mqttClient.connected()) {
        reconnectMQTT();
    }
    mqttClient.loop();
}

/**
 * @brief Publishes a message to the specified MQTT topic.
 * 
 * @param topic Topic name to publish to.
 * @param message Null-terminated string message to send.
 * @return true if published successfully, false otherwise.
 */
bool publishMessage(const char* topic, const char* message) {
    if (!mqttClient.connected()) {
        Serial.println("[MQTT] Cannot publish: not connected");
        return false;
    }

    bool success = mqttClient.publish(topic, message);
    if (success) {
        Serial.printf("[MQTT] Published to %s: %s\n", topic, message);
    } else {
        Serial.printf("[MQTT] Failed to publish to %s\n", topic);
    }
    return success;
}
