#ifndef SETTINGS_H
#define SETTINGS_H


// Network Configuration
struct NetworkConfig {
    // WiFi Settings
    static constexpr const char* WIFI_SSID = "Roro3";
    static constexpr const char* WIFI_PASSWORD = "rorororo";
    static constexpr int WIFI_TIMEOUT_MS = 10000; // Connection timeout (ms)
    static constexpr int WIFI_RETRY_DELAY = 500;  // Delay between retries (ms)

    // MQTT Settings
    static constexpr const char* MQTT_BROKER = "192.168.238.21";
    static constexpr int MQTT_PORT = 1883;
    static constexpr const char* MQTT_CLIENT_ID = "ESP32-Robot";
};

// Audio Configuration
struct AudioConfig {
    static const int SAMPLING_RATE = 44100;      // Audio sampling rate (Hz)
    static const int VOLUME_THRESHOLD = 1;    // Voice detection threshold (in % of max volume)
    static const int AVERAGING_SAMPLES = 64;     // Samples for moving average
};

// Hardware Configuration
struct PinConfig {
    // I2S Pins (INMP441 Microphone)
    static const int I2S_SCK = 37;    // Serial Clock (BCLK)
    static const int I2S_WS = 36;     // Word Select (LRCL)
    static const int I2S_SD = 38;     // Serial Data
    
    // Indicator LEDs
    static const int VOICE_ACTIVITY_LED = 7;  // Voice detection indicator

    // Display Pins (if using I2C OLED)
    static const int DISPLAY_SDA = 21;  // I2C Data
    static const int DISPLAY_SCL = 22;  // I2C Clock
};

// System Constants
struct SystemConfig {
    static const int SERIAL_BAUD_RATE = 115200;
    static const int MQTT_BUFFER_SIZE = 50000;  // MQTT message buffer size
};

// Task Configuration
struct TaskConfig {
    static const int ROBO_EYES_DELAY_MS = 10;
    static const int SERVO_UPDATE_DELAY_MS = 20;
    static const int MIC_UPDATE_DELAY_MS = 10;
    static const int WIFI_RETRY_INTERVAL_MS = 10000;
    static const int MQTT_HANDLE_DELAY_MS = 50;
    static const int SENSOR_SEND_INTERVAL_MS = 120000;  // 2 minutes
    static const int HEAP_CHECK_INTERVAL_MS = 5000;
    static const int SERIAL_INIT_TIMEOUT_MS = 2000;
};

#endif // SETTINGS_H
