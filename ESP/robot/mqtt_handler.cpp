#include "mqtt_handler.h"
#include "settings.h"

WiFiClient espClient;
PubSubClient mqttClient(espClient);

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
        
        // Handle incoming messages
        if (strcmp(topic, MQTT_TOPIC_COMMAND) == 0) {
            // TODO: Implement command handling
            Serial.println("[MQTT] Command received, processing...");
        }
    });

    // Initial connection attempt
    reconnectMQTT();
}

void reconnectMQTT() {
    int attempts = 0;
    while (!mqttClient.connected() && attempts < 3) {
        Serial.println("[MQTT] Attempting to connect...");
        
        if (mqttClient.connect(mqtt_client_id)) {
            Serial.println("[MQTT] Connected successfully");
            
            // Subscribe to command topic
            mqttClient.subscribe(MQTT_TOPIC_COMMAND);
            Serial.printf("[MQTT] Subscribed to %s\n", MQTT_TOPIC_COMMAND);
            
            // Publish online status
            mqttClient.publish(MQTT_TOPIC_STATUS, "online");
        } else {
            attempts++;
            Serial.print("[MQTT] Connection failed, rc=");
            Serial.print(mqttClient.state());
            Serial.printf(" (Attempt %d/3)\n", attempts);
            delay(1000);
        }
    }
}

void handleMQTT() {
    if (!mqttClient.connected()) {
        reconnectMQTT();
    }
    mqttClient.loop();
}

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
