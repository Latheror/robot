#include "INMP441.h"

INMP441::INMP441(int sampleRate) : _sampleRate(sampleRate) {}

void INMP441::setClapCallback(std::function<void()> callback) {
    _clapCallback = callback;
}

void INMP441::setIsRecordingCallback(std::function<void(bool)> callback) {
    _isRecordingCallback = callback;
}

bool INMP441::begin() {
    if (!configureI2S()) return false;

    pinMode(PinConfig::VOICE_ACTIVITY_LED, OUTPUT);
    digitalWrite(PinConfig::VOICE_ACTIVITY_LED, LOW);

    Serial.println("[MIC] INMP441 initialized successfully");
    return true;
}

/**
 * @brief Configures the I2S interface for the microphone.
 * @return true if successful, false otherwise.
 */
bool INMP441::configureI2S() const {
    Serial.println("[MIC] Configuring I2S...");

    i2s_config_t config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = _sampleRate,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S_MSB,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = SystemConfig::DMA_BUF_LEN,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pins = {
        .mck_io_num = I2S_PIN_NO_CHANGE,
        .bck_io_num = PinConfig::I2S_SCK,
        .ws_io_num = PinConfig::I2S_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = PinConfig::I2S_SD
    };

    esp_err_t err = i2s_driver_install(_i2sPort, &config, 0, NULL);
    if (err != ESP_OK) {
        Serial.printf("[MIC] I2S driver installation failed: %d\n", err);
        return false;
    }

    err = i2s_set_pin(_i2sPort, &pins);
    if (err != ESP_OK) {
        Serial.printf("[MIC] I2S pin configuration failed: %d\n", err);
        return false;
    }

    Serial.printf("[MIC] I2S configured successfully (BCK=%d, WS=%d, SD=%d)\n",
                  pins.bck_io_num, pins.ws_io_num, pins.data_in_num);
    return true;
}

void INMP441::update() {
    unsigned long now = millis();
    if (now - _lastUpdate < INMP441::UPDATE_INTERVAL) return;
    _lastUpdate = now;

    readSamplesAndComputeVolume();
    updateActivityLed();
    detectDoubleClap();
}

/**
 * @brief Reads samples from I2S and computes the current volume.
 */
void INMP441::readSamplesAndComputeVolume() {
    const int NUM_SAMPLES = SystemConfig::MIC_NUM_SAMPLES;
    int32_t buffer[NUM_SAMPLES];
    size_t bytesRead = 0;

    // Use 100ms timeout instead of portMAX_DELAY to prevent indefinite blocking
    const TickType_t timeout = pdMS_TO_TICKS(SystemConfig::MIC_READ_TIMEOUT_MS);
    esp_err_t res = i2s_read(_i2sPort, (char*)buffer, sizeof(buffer), &bytesRead, timeout);
    
    if (res != ESP_OK) {
        if (res == ESP_ERR_TIMEOUT) {
            Serial.println("[MIC] I2S read timeout - hardware may be stuck");
        } else {
            Serial.printf("[MIC] I2S read error: %d\n", res);
        }
        return;
    }
    
    if (bytesRead == 0) return;

    int samplesRead = bytesRead / sizeof(int32_t);
    
    // Guard against division by zero
    if (samplesRead <= 0) {
        Serial.println("[MIC] Warning: samplesRead is 0 or negative");
        _currentVolume = 0.0;
        return;
    }
    
    double sumSquares = 0.0;

    for (int i = 0; i < samplesRead; i++) {
        int32_t aligned = buffer[i] >> 8;
        double normalized = (double)aligned / INMP441::MAX_24BIT;
        sumSquares += normalized * normalized;
    }

    _currentVolume = sqrt(sumSquares / samplesRead);
}

/**
 * @brief Updates the activity LED based on volume threshold.
 */
void INMP441::updateActivityLed() {
    digitalWrite(PinConfig::VOICE_ACTIVITY_LED, isVolumeAboveThreshold() ? HIGH : LOW);
}

/**
 * @brief Detects double clap events.
 */
void INMP441::detectDoubleClap() {
    if (!_clapDetectionEnabled) return;
    
    unsigned long now = millis();

    if (_currentVolume * 100 <= INMP441::CLAP_THRESHOLD) return;
    if (now - _lastClapTime <= INMP441::CLAP_DEBOUNCE) return;

    if (_firstClapTime == 0) {
        _firstClapTime = now;
        Serial.println("[MIC] First clap detected");
    } else {
        if (now - _firstClapTime <= INMP441::DOUBLE_CLAP_MAX_DELAY) {
            Serial.println("[MIC] Double clap detected!");
            if (_clapCallback) _clapCallback();
        }
        _firstClapTime = 0;
    }

    _lastClapTime = now;

    if (_firstClapTime && (now - _firstClapTime) > INMP441::DOUBLE_CLAP_MAX_DELAY)
        _firstClapTime = 0;
}

int INMP441::readSamplesForRecording(int32_t* buffer, int numSamples) {
    size_t bytesToRead = numSamples * sizeof(int32_t);
    size_t bytesRead = 0;
    
    esp_err_t res = i2s_read(_i2sPort, (char*)buffer, bytesToRead, &bytesRead, pdMS_TO_TICKS(100));
    if (res != ESP_OK) {
        return 0;
    }
    
    int samplesRead = bytesRead / sizeof(int32_t);
    // Convert to 24-bit samples
    for (int i = 0; i < samplesRead; i++) {
        buffer[i] = buffer[i] >> SystemConfig::MIC_BIT_SHIFT;
    }
    return samplesRead;
}

float INMP441::getVolume() const {
    return _currentVolume;
}

bool INMP441::isVolumeAboveThreshold() const {
    return _currentVolume > AudioConfig::VOLUME_THRESHOLD;
}
