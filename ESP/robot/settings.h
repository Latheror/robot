#ifndef SETTINGS_H
#define SETTINGS_H

// WiFi settings
static const char* ssid = "Roro3";
static const char* password = "rorororo";

// MQTT Broker settings
static const char* mqtt_broker = "192.168.246.21s";  // Your PC's IP address
static const int mqtt_port = 1883;                 // Default MQTT port
static const char* mqtt_client_id = "ESP32-Robot";  // Client ID for MQTT connection

// Audio settings
static const int AUDIO_SAMPLING_RATE = 44100;
static const int SOUND_THRESHOLD = 1000;      // Sound detection threshold (adjust based on testing)
static const int SOUND_AVERAGING_SAMPLES = 64; // Number of samples for moving average

// Hardware pins
static const int SOUND_ACTIVITY_LED_PIN = 7;  // LED pin for sound activity indicator

#endif // SETTINGS_H
