#ifndef SETTINGS_H
#define SETTINGS_H

// WiFi settings
// TODO: Replace with your actual credentials before uploading
const char* ssid = "";
const char* password = "";

// MQTT Broker settings
const char* mqtt_broker = "192.168.1.XXX";  // Replace with your PC's IP address
const int mqtt_port = 1883;                 // Default MQTT port
const char* mqtt_client_id = "ESP8266-Robot";  // Client ID for MQTT connection

#endif // SETTINGS_H
