#include "INMP441.h"

INMP441::INMP441(int sampleRate) {
    _sampleRate = sampleRate;
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
        Serial.println("Erreur installation I2S !");
        return false;
    }

    err = i2s_set_pin(_i2sPort, &pin_config);
    if (err != ESP_OK) {
        Serial.println("Erreur configuration pins I2S !");
        return false;
    }

    Serial.println("INMP441 initialisé avec succès !");
    return true;
}

int32_t INMP441::readSample() {
    int16_t sample = 0;
    size_t bytes_read = 0;
    i2s_read(_i2sPort, &sample, sizeof(sample), &bytes_read, portMAX_DELAY);
    return sample;
}
