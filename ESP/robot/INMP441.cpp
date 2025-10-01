#include "INMP441.h"

INMP441::INMP441(int sampleRate) {
    _sampleRate = sampleRate;
    memset(_samples, 0, sizeof(_samples));
}

bool INMP441::begin() {
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

    i2s_pin_config_t pin_config = {
        .bck_io_num = _pinBCLK,
        .ws_io_num = _pinLRCL,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = _pinDOUT
    };

    esp_err_t err;
    err = i2s_driver_install(_i2sPort, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        Serial.println("I2S installation error!");
        return false;
    }

    err = i2s_set_pin(_i2sPort, &pin_config);
    if (err != ESP_OK) {
        Serial.println("I2S pin configuration error!");
        return false;
    }

    // Configure sound detection LED
    pinMode(SOUND_ACTIVITY_LED_PIN, OUTPUT);
    digitalWrite(SOUND_ACTIVITY_LED_PIN, LOW);

    Serial.println("INMP441 initialized successfully!");
    return true;
}

int32_t INMP441::readSample() {
    int16_t sample = 0;
    size_t bytes_read = 0;
    i2s_read(_i2sPort, &sample, sizeof(sample), &bytes_read, portMAX_DELAY);
    return sample;
}

void INMP441::update() {
    unsigned long currentTime = millis();
    if (currentTime - _lastUpdate >= UPDATE_INTERVAL) {
        _lastUpdate = currentTime;
        
        // Add new sample
        int32_t rawSample = readSample();
        _samples[_sampleIndex] = abs(rawSample);
        _sampleIndex = (_sampleIndex + 1) % SOUND_AVERAGING_SAMPLES;

        // Calculate new volume
        calculateVolume();

        // Update LED
        bool isAboveThreshold = isVolumeAboveThreshold();
        digitalWrite(SOUND_ACTIVITY_LED_PIN, isAboveThreshold ? HIGH : LOW);

        // Print debug information periodically
        if (currentTime - _lastDebugPrint >= DEBUG_INTERVAL) {
            _lastDebugPrint = currentTime;
            Serial.printf("Sound Debug - Raw: %d, Volume: %.2f, Threshold: %d, LED: %s\n", 
                         rawSample, 
                         _currentVolume, 
                         SOUND_THRESHOLD,
                         isAboveThreshold ? "ON" : "OFF");
            Serial.printf("LED Pin: %d, Pin State: %d\n", 
                         SOUND_ACTIVITY_LED_PIN, 
                         digitalRead(SOUND_ACTIVITY_LED_PIN));
        }
    }
}

void INMP441::calculateVolume() {
    float sum = 0;
    for (int i = 0; i < SOUND_AVERAGING_SAMPLES; i++) {
        sum += _samples[i];
    }
    _currentVolume = sum / SOUND_AVERAGING_SAMPLES;
}

float INMP441::getVolume() {
    return _currentVolume;
}

bool INMP441::isVolumeAboveThreshold() {
    return _currentVolume > SOUND_THRESHOLD;
}
