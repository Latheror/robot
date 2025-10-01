#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <WiFi.h>
#include <PubSubClient.h>

// MQTT Topics
const char* const MQTT_TOPIC_SENSORS = "robot/1/sensors";
const char* const MQTT_TOPIC_COMMANDS = "robot/1/commands";
const char* const MQTT_TOPIC_AUDIO = "robot/1/audio";

// Public functions
bool setupMQTT();        // Initialize MQTT connection
void handleMQTT();       // Process MQTT events, maintain connection
bool publishMessage(const char* topic, const char* message); // Publish a message to a topic

#endif // MQTT_HANDLER_H
