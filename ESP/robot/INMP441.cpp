#include "INMP441.h"

// Constructor
INMP441::INMP441(int sampleRate) : _sampleRate(sampleRate) {}

// Set clap callback
void INMP441::setClapCallback(std::function<void()> callback) {
    _clapCallback = callback;
}

// Initialize microphone
bool INMP441::begin() {
    if (!configureI2S()) return false;

    pinMode(PinConfig::VOICE_ACTIVITY_LED, OUTPUT);
    digitalWrite(PinConfig::VOICE_ACTIVITY_LED, LOW);

    Serial.println("[MIC] INMP441 initialized successfully");
    return true;
}

// I2S configuration (unchanged)
bool INMP441::configureI2S() const {
    Serial.println("[MIC] Configuring I2S...");
    
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = _sampleRate,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S_MSB,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
        .mck_io_num = I2S_PIN_NO_CHANGE,
        .bck_io_num = PinConfig::I2S_SCK,
        .ws_io_num = PinConfig::I2S_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = PinConfig::I2S_SD
    };

    esp_err_t err = i2s_driver_install(_i2sPort, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        Serial.printf("[MIC] I2S driver installation failed: %d\n", err);
        return false;
    }
    Serial.println("[MIC] I2S driver installed successfully");

    err = i2s_set_pin(_i2sPort, &pin_config);
    if (err != ESP_OK) {
        Serial.printf("[MIC] I2S pin configuration failed: %d\n", err);
        return false;
    }
    Serial.printf("[MIC] I2S pins configured: BCK=%d, WS=%d, SD=%d\n", 
                 pin_config.bck_io_num,
                 pin_config.ws_io_num,
                 pin_config.data_in_num);

    return true;
}

// Update microphone samples and volume
void INMP441::update() {
    unsigned long currentTime = millis();
    if (currentTime - _lastUpdate >= UPDATE_INTERVAL) {
        _lastUpdate = currentTime;

        const int NUM_SAMPLES = 256;
        int32_t buffer[NUM_SAMPLES];
        size_t bytes_read = 0;

        esp_err_t res = i2s_read(_i2sPort, (char*)buffer, sizeof(buffer), &bytes_read, portMAX_DELAY);
        if (res != ESP_OK || bytes_read == 0) return;

        int samples_read = bytes_read / sizeof(int32_t);
        double sumSquares = 0.0;
        int32_t lastRawSample = 0;

        for (int i = 0; i < samples_read; i++) {
            int32_t aligned = buffer[i] >> 8;
            lastRawSample = aligned;
            double normalized = (double)aligned / MAX_24BIT;
            sumSquares += normalized * normalized;
        }

        _currentVolume = sqrt(sumSquares / samples_read);

        digitalWrite(PinConfig::VOICE_ACTIVITY_LED, isVolumeAboveThreshold() ? HIGH : LOW);

        if (currentTime - _lastDebugPrint >= DEBUG_INTERVAL) {
            _lastDebugPrint = currentTime;
            // Serial.printf("[MIC] Raw: %d, Vol: %.2f%%, Thresh: %d%%, Active: %s\n", 
            //               lastRawSample,
            //               _currentVolume * 100.0,
            //               AudioConfig::VOLUME_THRESHOLD,
            //               isVolumeAboveThreshold() ? "Yes" : "No");
        }

        // Check for clap
        unsigned long now = millis();
        if (_currentVolume * 100 > CLAP_THRESHOLD && (now - _lastClapTime) > CLAP_DEBOUNCE) {
            _lastClapTime = now;
            if (_clapCallback) _clapCallback();
            Serial.println("[MIC] Clap detected!");
        }
    }
}

// Read a single sample
int32_t INMP441::readSample() {
    int32_t sample = 0;
    size_t bytes_read = 0;

    if (i2s_read(_i2sPort, &sample, sizeof(sample), &bytes_read, portMAX_DELAY) == ESP_OK) {
        return sample >> 8;  // 24 valid bits
    }
    return 0;
}

float INMP441::getVolume() const {
    return _currentVolume;
}

bool INMP441::isVolumeAboveThreshold() const {
    return (_currentVolume * 100.0) > AudioConfig::VOLUME_THRESHOLD;
}
