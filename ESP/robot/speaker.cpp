#include "Speaker.h"
#include <driver/i2s.h>
#include <math.h>
#include "FS.h"
#include <LittleFS.h>

#define SAMPLE_RATE 24000
#define I2S_NUM I2S_NUM_0

/**
 * @brief Constructor for the Speaker class.
 *        All initialization is done in init().
 */
Speaker::Speaker() {
    // Empty constructor
}

/**
 * @brief Initialize the speaker system and mount LittleFS.
 */
void Speaker::init() {
    // Mount LittleFS filesystem
    if (!LittleFS.begin(true)) {
        Serial.println("[Speaker] Failed to mount LittleFS");
        return;
    }

    Serial.println("[Speaker] LittleFS mounted successfully");
    Serial.printf("[Speaker] Total: %llu, Used: %llu bytes\n", LittleFS.totalBytes(), LittleFS.usedBytes());

    // Initialize I2S peripheral
    i2sInit();
}

/**
 * @brief Configure I2S peripheral for audio output.
 */
void Speaker::i2sInit() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
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

/**
 * @brief Play a simple sine tone.
 * @param frequency Frequency of the tone in Hz.
 * @param durationMs Duration of the tone in milliseconds.
 * @param volume Volume from 0.0 (mute) to 1.0 (max).
 */
void Speaker::playTone(float frequency, int durationMs, float volume) {
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;

    int sampleCount = (SAMPLE_RATE * durationMs) / 1000;
    int16_t buffer[256];
    int index = 0;

    const int16_t maxAmplitude = 32767;

    for (int i = 0; i < sampleCount; i++) {
        float sample = sinf(2 * M_PI * frequency * i / SAMPLE_RATE);
        buffer[index++] = (int16_t)(sample * maxAmplitude * volume);

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

/**
 * @brief Play an example sequence of tones.
 */
void Speaker::playExampleSound() {
    playTone(440.0, 300, 0.1f);  // A4
    delay(50);
    playTone(660.0, 300, 0.1f);  // E5
    delay(50);
    playTone(880.0, 500, 0.1f);  // A5
    delay(200);
}

/**
 * @brief Play a WAV file from LittleFS.
 * @param path Path to the WAV file in LittleFS.
 */
void Speaker::playWav(const char* path) {
    File file = LittleFS.open(path);
    if (!file) {
        Serial.println("[Speaker] Failed to open WAV file!");
        return;
    }

    Serial.println("[Speaker] WAV file opened successfully");

    // Skip typical WAV header (44 bytes)
    file.seek(44);

    uint8_t buffer[512];
    size_t bytesRead, bytesWritten;

    while ((bytesRead = file.read(buffer, sizeof(buffer))) > 0) {
        i2s_write(I2S_NUM, buffer, bytesRead, &bytesWritten, portMAX_DELAY);
    }

    file.close();
}

/**
 * @brief List example files in LittleFS.
 */
void Speaker::listFiles() {
    const char* files[] = {"/start_speech.wav"};
    Serial.println("[Speaker] Listing files:");
    for (int i = 0; i < sizeof(files)/sizeof(files[0]); i++) {
        if (LittleFS.exists(files[i])) {
            Serial.print("[Speaker] Found: ");
            Serial.println(files[i]);
        }
    }
}

/**
 * @brief Play an entire buffer in memory via I2S.
 * @param buffer Pointer to the audio buffer.
 * @param len Length of the buffer in bytes.
 */
void Speaker::playWavFromBuffer(uint8_t* buffer, size_t len) {
    Serial.printf("[Speaker] playWavFromBuffer: %d bytes\n", (int)len);

    size_t chunkSize = 512;
    size_t offset = 0;
    while (offset < len) {
        size_t toWrite = (len - offset) > chunkSize ? chunkSize : (len - offset);
        size_t bytesWritten;
        i2s_write(I2S_NUM, buffer + offset, toWrite, &bytesWritten, portMAX_DELAY);
        offset += bytesWritten;
    }

    Serial.println("[Speaker] Finished playing buffer");
}

/**
 * @brief Play a decoded chunk of audio via I2S (streaming).
 *        This is useful for MQTT chunked audio playback.
 * @param buffer Pointer to the chunk buffer.
 * @param len Length of the chunk in bytes.
 */
void Speaker::playWavChunk(const uint8_t* buffer, size_t len) {
    size_t offset = 0;
    while (offset < len) {
        size_t toWrite = len - offset;
        size_t bytesWritten;
        i2s_write(I2S_NUM, buffer + offset, toWrite, &bytesWritten, portMAX_DELAY);
        offset += bytesWritten;
    }
}
