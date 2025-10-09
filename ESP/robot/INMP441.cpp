#include "INMP441.h"

unsigned long _firstClapTime = 0;
const unsigned long DOUBLE_CLAP_MAX_DELAY = 500; // ms between claps

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
        .dma_buf_len = 64,
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
    if (now - _lastUpdate < UPDATE_INTERVAL) return;
    _lastUpdate = now;

    readSamplesAndComputeVolume();
    updateActivityLed();
    detectDoubleClap();

    // If recording, append sample to buffer
    if (_recording) {
        int32_t sample = readSample();
        _recordBuffer.push_back(sample);
    }
}

void INMP441::readSamplesAndComputeVolume() {
    const int NUM_SAMPLES = 256;
    int32_t buffer[NUM_SAMPLES];
    size_t bytesRead = 0;

    esp_err_t res = i2s_read(_i2sPort, (char*)buffer, sizeof(buffer), &bytesRead, portMAX_DELAY);
    if (res != ESP_OK || bytesRead == 0) return;

    int samplesRead = bytesRead / sizeof(int32_t);
    double sumSquares = 0.0;

    for (int i = 0; i < samplesRead; i++) {
        int32_t aligned = buffer[i] >> 8;
        double normalized = (double)aligned / MAX_24BIT;
        sumSquares += normalized * normalized;

        if (_recording) _recordBuffer.push_back(aligned);
    }

    _currentVolume = sqrt(sumSquares / samplesRead);
}

void INMP441::updateActivityLed() {
    digitalWrite(PinConfig::VOICE_ACTIVITY_LED, isVolumeAboveThreshold() ? HIGH : LOW);
}

void INMP441::detectDoubleClap() {
    unsigned long now = millis();

    if (_currentVolume <= CLAP_THRESHOLD) return;
    if (now - _lastClapTime <= CLAP_DEBOUNCE) return;

    if (_firstClapTime == 0) {
        _firstClapTime = now;
        Serial.println("[MIC] First clap detected");
    } else {
        if (now - _firstClapTime <= DOUBLE_CLAP_MAX_DELAY) {
            Serial.println("[MIC] Double clap detected!");
            if (_clapCallback) _clapCallback();
        }
        _firstClapTime = 0;
    }

    _lastClapTime = now;

    if (_firstClapTime && (now - _firstClapTime) > DOUBLE_CLAP_MAX_DELAY)
        _firstClapTime = 0;
}

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
    return _currentVolume > AudioConfig::VOLUME_THRESHOLD;
}

// ---- Recording functions ----

void INMP441::startRecording() {
    _recordBuffer.clear();
    _recording = true;
    Serial.println("[MIC] Recording started");
}

void INMP441::stopRecording() {
    _recording = false;
    Serial.printf("[MIC] Recording stopped, %d samples captured\n", (int)_recordBuffer.size());
}

const std::vector<int32_t>& INMP441::getBuffer() const {
    return _recordBuffer;
}
