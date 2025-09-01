#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Function declarations
void setupMQTT();
void reconnectMQTT();
void handleMQTT();
bool publishMessage(const char* topic, const char* message);

// Topic definitions
#define MQTT_TOPIC_STATUS "robot/status"
#define MQTT_TOPIC_POSITION "robot/position"
#define MQTT_TOPIC_COMMAND "robot/command"

#endif // MQTT_HANDLER_H
