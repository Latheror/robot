#include "INMP441.h"

INMP441::INMP441(int sampleRate) : _sampleRate(sampleRate) {}

bool INMP441::begin() {
    // Configure I2S interface
    if (!configureI2S()) {
        return false;
    }

    // Setup activity LED
    pinMode(PinConfig::VOICE_ACTIVITY_LED, OUTPUT);
    digitalWrite(PinConfig::VOICE_ACTIVITY_LED, LOW);

    Serial.println("[MIC] INMP441 initialized successfully");
    return true;
}

bool INMP441::configureI2S() const {
    // I2S configuration
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = _sampleRate,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S_MSB,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 1024,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    // Pin configuration
    i2s_pin_config_t pin_config = {
        .bck_io_num = PinConfig::I2S_SCK,
        .ws_io_num = PinConfig::I2S_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = PinConfig::I2S_SD
    };

    // Initialize I2S
    if (i2s_driver_install(_i2sPort, &i2s_config, 0, NULL) != ESP_OK) {
        Serial.println("[MIC] I2S driver installation failed");
        return false;
    }

    if (i2s_set_pin(_i2sPort, &pin_config) != ESP_OK) {
        Serial.println("[MIC] I2S pin configuration failed");
        return false;
    }

    return true;
}

void INMP441::update() {
    unsigned long currentTime = millis();
    if (currentTime - _lastUpdate >= UPDATE_INTERVAL) {
        _lastUpdate = currentTime;
        
        // Process new sample
        int32_t rawSample = readSample();
        _samples[_sampleIndex] = abs(rawSample);
        _sampleIndex = (_sampleIndex + 1) % AudioConfig::AVERAGING_SAMPLES;

        // Update volume and LED
        calculateVolume();
        digitalWrite(PinConfig::VOICE_ACTIVITY_LED, isVolumeAboveThreshold() ? HIGH : LOW);

        // Debug output
        if (currentTime - _lastDebugPrint >= DEBUG_INTERVAL) {
            _lastDebugPrint = currentTime;
            Serial.printf("[MIC] Raw: %d, Vol: %.2f, Thresh: %d, Active: %s\n", 
                         rawSample, 
                         _currentVolume, 
                         AudioConfig::VOLUME_THRESHOLD,
                         isVolumeAboveThreshold() ? "Yes" : "No");
        }
    }
}

int32_t INMP441::readSample() {
    int16_t sample = 0;
    size_t bytes_read = 0;
    i2s_read(_i2sPort, &sample, sizeof(sample), &bytes_read, portMAX_DELAY);
    return sample;
}

void INMP441::calculateVolume() {
    float sum = 0.0f;
    for (const auto& sample : _samples) {
        sum += sample;
    }
    _currentVolume = sum / AudioConfig::AVERAGING_SAMPLES;
}

float INMP441::getVolume() const {
    return _currentVolume;
}

bool INMP441::isVolumeAboveThreshold() const {
    return _currentVolume > AudioConfig::VOLUME_THRESHOLD;
}
