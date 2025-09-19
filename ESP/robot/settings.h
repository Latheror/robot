#ifndef SETTINGS_H
#define SETTINGS_H

// WiFi settings
static const char* ssid = "Roro3";
static const char* password = "rorororo";

// MQTT Broker settings
static const char* mqtt_broker = "192.168.226.21";  // Your PC's IP address
static const int mqtt_port = 1883;                 // Default MQTT port
static const char* mqtt_client_id = "ESP32-Robot";  // Client ID for MQTT connection

#endif // SETTINGS_H
