#ifndef SETTINGS_H
#define SETTINGS_H

/**
 * @brief Network configuration settings.
 */
struct NetworkConfig {
    // WiFi Settings
    static constexpr const char* WIFI_SSID = "RoroMeme";           ///< WiFi SSID
    static constexpr const char* WIFI_PASSWORD = "roromemewifi3";    ///< WiFi password
    static constexpr int WIFI_TIMEOUT_MS = 10000;              ///< Connection timeout (ms)
    static constexpr int WIFI_RETRY_DELAY = 500;               ///< Delay between retries (ms)
    static const int WIFI_LOG_INTERVAL_MS = 1000;              ///< WiFi connection log interval

    // MQTT Settings
    static constexpr const char* MQTT_BROKER = "192.168.1.210"; ///< MQTT broker IP
    static constexpr int MQTT_PORT = 1883;                       ///< MQTT port
    static constexpr const char* MQTT_CLIENT_ID = "ESP32-Robot"; ///< MQTT client ID
};

/**
 * @brief Audio configuration settings.
 */
struct AudioConfig {
    static const int SAMPLING_RATE = 16000;      ///< Audio sampling rate (Hz)
    static constexpr float VOLUME_THRESHOLD = 0.1f;       ///< Voice detection threshold (in % of max volume)
    static const int AVERAGING_SAMPLES = 64;     ///< Samples for moving average
};

/**
 * @brief Hardware pin configuration.
 */
struct PinConfig {
    // I2S Pins (INMP441 Microphone)
    static const int I2S_SCK = 4;    ///< Serial Clock (BCLK)
    static const int I2S_WS = 5;     ///< Word Select (LRCL)
    static const int I2S_SD = 6;     ///< Serial Data

    // Indicator LEDs
    static const int VOICE_ACTIVITY_LED = 7;  ///< Voice detection indicator

    // I2C Pins
    static const int I2C_SDA = 17;  ///< I2C Data
    static const int I2C_SCL = 18;  ///< I2C Clock

    // LED Strip
    static const int LED_STRIP_PIN = 13;  ///< LED strip data pin
};

/**
 * @brief OLED display and LED strip configuration settings.
 */
struct OledDisplayConfig {
    static const int SCREEN_WIDTH = 128;   ///< OLED screen width
    static const int SCREEN_HEIGHT = 64;   ///< OLED screen height
    static const int OLED_ADDR = 0x3C;     ///< OLED I2C address
    static const int LED_BRIGHTNESS = 3;   ///< Default LED brightness
    static const int STRIP_LED_COUNT = 5;  ///< Number of LEDs in strip
};

struct SystemConfig {
    static const int SERIAL_BAUD_RATE = 115200;   ///< Serial baud rate
    static const int MQTT_BUFFER_SIZE = 50000;    ///< MQTT message buffer size
    static const int MQTT_MAX_MESSAGE_SIZE = 16384; ///< Maximum MQTT message size
    static const int JSON_DOC_SIZE = 512;         ///< Default JSON document size
    static const int AUDIO_CHUNK_SIZE = 256;      ///< Audio processing chunk size
    static const int AUDIO_BUFFER_SIZE = 512;     ///< Audio processing buffer size
    static const int WAV_HEADER_SIZE = 44;        ///< WAV file header size
    static const int16_t MAX_AMPLITUDE = 32767;   ///< Maximum audio amplitude
    static const int DMA_BUF_LEN = 64;            ///< I2S DMA buffer length
    static const int MIC_BIT_SHIFT = 8;           ///< Microphone bit shift for 24-bit to 16-bit
    static const int MIC_NUM_SAMPLES = 256;       ///< Microphone sample count
    static const int MIC_READ_TIMEOUT_MS = 100;   ///< Microphone I2S read timeout
    static const int MUTEX_TIMEOUT_MS = 100;      ///< Mutex wait timeout
    static const int RECONNECT_DELAY_MS = 5000;   ///< MQTT reconnect delay
    static const int AUDIO_JSON_DOC_SIZE = 16384; ///< JSON document size for audio messages
};

/**
 * @brief Task timing configuration.
 */
struct TaskConfig {
    static const int ROBO_EYES_DELAY_MS = 10;         ///< RoboEyes update delay
    static const int SERVO_UPDATE_DELAY_MS = 20;      ///< Servo update delay
    static const int MIC_UPDATE_DELAY_MS = 10;        ///< Microphone update delay
    static const int WIFI_RETRY_INTERVAL_MS = 10000;  ///< WiFi retry interval
    static const int MQTT_HANDLE_DELAY_MS = 50;       ///< MQTT handle delay
    static const int SENSOR_SEND_INTERVAL_MS = 120000; ///< Sensor send interval (2 minutes)
    static const int HEAP_CHECK_INTERVAL_MS = 5000;    ///< Heap check interval
    static const int MQTT_RECONNECT_DELAY_MS = 1000; ///< Delay after MQTT reconnect
    static const int SERIAL_INIT_TIMEOUT_MS = 2000;    ///< Serial init timeout
};

#endif // SETTINGS_H
