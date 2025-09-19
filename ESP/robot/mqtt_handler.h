#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <WiFi.h>
#include <PubSubClient.h>

// Function declarations
void setupMQTT();
void reconnectMQTT();
void handleMQTT();
bool publishMessage(const char* topic, const char* message);

// Topic definitions
#define MQTT_TOPIC_SENSORS "robot/1/sensors"

#endif // MQTT_HANDLER_H
