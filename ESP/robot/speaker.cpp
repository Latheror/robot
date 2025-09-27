#include "Speaker.h"
#include <driver/i2s.h>
#include <math.h>

#define SAMPLE_RATE 44100
#define I2S_NUM I2S_NUM_0

Speaker::Speaker() {
    // Empty constructor; everything is in init()
}

void Speaker::init() {
    i2sInit();
}

void Speaker::i2sInit() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
        .intr_alloc_flags = 0,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = BCLK_PIN,
        .ws_io_num = LRCK_PIN,
        .data_out_num = DATA_PIN,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM, &pin_config);
}

void Speaker::playTone(float frequency, int durationMs) {
    int sampleCount = (SAMPLE_RATE * durationMs) / 1000;
    int16_t buffer[256];
    int index = 0;

    for (int i = 0; i < sampleCount; i++) {
        buffer[index++] = (int16_t)(sin(2 * PI * frequency * i / SAMPLE_RATE) * 30000); // amplitude
        if (index == 256) {
            size_t bytesWritten;
            i2s_write(I2S_NUM, buffer, sizeof(buffer), &bytesWritten, portMAX_DELAY);
            index = 0;
        }
    }

    if (index > 0) {
        size_t bytesWritten;
        i2s_write(I2S_NUM, buffer, index * sizeof(int16_t), &bytesWritten, portMAX_DELAY);
    }
}

void Speaker::playExampleSound() {
    // Simple 3-note melody
    playTone(440.0, 300);  // A4
    delay(50);
    playTone(660.0, 300);  // E5
    delay(50);
    playTone(880.0, 500);  // A5
    delay(200);
}