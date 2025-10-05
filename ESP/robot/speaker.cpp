#include "Speaker.h"
#include <LittleFS.h>
#include <math.h>

SemaphoreHandle_t audioMutex = nullptr;

bool Speaker::begin(const AudioConfig& config) {
    if (_initialized) return true;
    
    _config = config;
    
    if (!initFileSystem() || !initI2S()) {
        return false;
    }

    /* Init Mutex */
    audioMutex = xSemaphoreCreateMutex();
    if (audioMutex == nullptr) {
        Serial.println("[AUDIO] Failed to create audio mutex");
        return false;
    }
    
    _initialized = true;
    Serial.println("[AUDIO] Speaker initialized successfully");
    return true;
}

bool Speaker::initFileSystem() {
    if (!LittleFS.begin(true)) {
        Serial.println("[AUDIO] Failed to mount file system");
        return false;
    }
    
    Serial.printf("[AUDIO] Storage: %llu used / %llu total bytes\n",
                 LittleFS.usedBytes(), LittleFS.totalBytes());
    return true;
}

bool Speaker::initI2S() {
    const i2s_config_t config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = _config.sampleRate,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S_MSB,
        .intr_alloc_flags = 0,
        .dma_buf_count = _config.dmaBufCount,
        .dma_buf_len = _config.dmaBufLen,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };

    const i2s_pin_config_t pins = {
        .bck_io_num = BCLK_PIN,
        .ws_io_num = WS_PIN,
        .data_out_num = DATA_PIN,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    if (i2s_driver_install(_i2sPort, &config, 0, nullptr) != ESP_OK) {
        Serial.println("[AUDIO] Failed to install I2S driver");
        return false;
    }

    if (i2s_set_pin(_i2sPort, &pins) != ESP_OK) {
        Serial.println("[AUDIO] Failed to set I2S pins");
        return false;
    }

    return true;
}

bool Speaker::playTone(float frequency, uint32_t durationMs, float volume) {
    if (!_initialized || frequency <= 0 || durationMs == 0) return false;
    
    // Clamp volume to valid range
    volume = std::clamp(volume, 0.0f, 1.0f);
    
    // Calculate number of samples needed
    const size_t samplesNeeded = (_config.sampleRate * durationMs) / 1000;
    const size_t bufferSize = 256; // Process in chunks
    int16_t buffer[bufferSize];
    
    _playing = true;
    size_t samplesProcessed = 0;
    
    while (samplesProcessed < samplesNeeded) {
        size_t samplesToGenerate = std::min(bufferSize, samplesNeeded - samplesProcessed);
        
        // Generate tone samples
        generateTone(frequency, volume, buffer, samplesToGenerate, _config.sampleRate);
        
        // Write to I2S
        if (!writeSamples(buffer, samplesToGenerate * sizeof(int16_t))) {
            _playing = false;
            return false;
        }
        
        samplesProcessed += samplesToGenerate;
    }
    
    _playing = false;
    return true;
}

bool Speaker::playWav(const char* path, bool skipHeader) {

    Serial.printf("[AUDIO] Playing WAV file: %s\n", path);
    
    if (!_initialized || !path) return false;
    if (!audioMutex) return false;

    /* Wait for audio mutex */
    if (xSemaphoreTake(audioMutex, portMAX_DELAY) != pdTRUE) {
         Serial.println("[AUDIO] Failed to take audio mutex");
         return false;
    }

    File file = LittleFS.open(path);
    if (!file) {
        Serial.println("[AUDIO] Failed to open WAV file");
        xSemaphoreGive(audioMutex);
        return false;
    }
    
    // Skip WAV header if requested (typically 44 bytes)
    if (skipHeader) {
        file.seek(44);
    }
    
    _playing = true;
    uint8_t buffer[512];
    bool success = true;
    
    while (file.available()) {
        size_t bytesRead = file.read(buffer, sizeof(buffer));
        if (!writeSamples(buffer, bytesRead)) {
            success = false;
            break;
        }
    }
    
    file.close();
    _playing = false;

    /* Release audio mutex */
    xSemaphoreGive(audioMutex);
    
    return success;
}

bool Speaker::playBuffer(const uint8_t* buffer, size_t length) {
    if (!_initialized || !buffer || length == 0) return false;
    
    _playing = true;
    const size_t chunkSize = 512;
    bool success = true;
    
    for (size_t offset = 0; offset < length; offset += chunkSize) {
        size_t chunk = std::min(chunkSize, length - offset);
        if (!writeSamples(buffer + offset, chunk)) {
            success = false;
            break;
        }
    }
    
    _playing = false;
    return success;
}

bool Speaker::playChunk(const uint8_t* chunk, size_t length) {
    if (!_initialized || !chunk || length == 0) return false;
    return writeSamples(chunk, length);
}

void Speaker::stop() {
    if (_initialized) {
        i2s_zero_dma_buffer(_i2sPort);
        _playing = false;
    }
}

bool Speaker::isPlaying() const {
    return _playing;
}

bool Speaker::checkFile(const char* path) {
    return LittleFS.exists(path);
}

void Speaker::listFiles(const char* directory) {
    File root = LittleFS.open(directory);
    if (!root || !root.isDirectory()) {
        Serial.println("[AUDIO] Failed to open directory");
        return;
    }
    
    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            Serial.printf("[AUDIO] DIR: %s\n", file.path());
        } else {
            Serial.printf("[AUDIO] FILE: %s (%u bytes)\n", 
                         file.path(), file.size());
        }
        file = root.openNextFile();
    }
}

size_t Speaker::writeSamples(const void* buffer, size_t bytes) {
    size_t bytesWritten = 0;
    esp_err_t err = i2s_write(_i2sPort, buffer, bytes, &bytesWritten, portMAX_DELAY);
    return (err == ESP_OK) ? bytesWritten : 0;
}

void Speaker::generateTone(float frequency, float volume, 
                         int16_t* buffer, size_t samples,
                         uint32_t sampleRate) {
    static constexpr int16_t MAX_AMPLITUDE = 32767;
    constexpr float two_pi = 6.283185307179586476925286766559f;
    
    static float phase = 0.0f;
    float phaseIncrement = TWO_PI * frequency / sampleRate;
    
    for (size_t i = 0; i < samples; i++) {
        float sample = sinf(phase) * volume;
        buffer[i] = static_cast<int16_t>(sample * MAX_AMPLITUDE);
        
        phase += phaseIncrement;
        if (phase >= TWO_PI) {
            phase -= TWO_PI;
        }
    }
}
