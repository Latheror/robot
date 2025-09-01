#ifndef SETTINGS_H
#define SETTINGS_H

// WiFi settings
static const char* ssid = "";
static const char* password = "";

// MQTT Broker settings
static const char* mqtt_broker = "192.168.38.21";  // Your PC's IP address
static const int mqtt_port = 1883;                 // Default MQTT port
static const char* mqtt_client_id = "ESP8266-Robot";  // Client ID for MQTT connection

#endif // SETTINGS_H
