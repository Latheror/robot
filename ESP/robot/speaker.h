/**
 * @file speaker.h
 * @brief I2S speaker playback, LittleFS audio file access, and shared audio mutex utilities.
 */

#ifndef SPEAKER_H
#define SPEAKER_H

#include <Arduino.h>
#include <driver/i2s.h>
#include "FS.h"
#include "settings.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

/**
 * @class MutexGuard
 * @brief RAII wrapper for FreeRTOS semaphore/mutex
 * 
 * Automatically acquires mutex on construction and releases on destruction.
 * Prevents deadlock and ensures mutex is released even if function returns early.
 */
class MutexGuard {
public:
    /**
     * @brief Acquire mutex with optional timeout
     * @param mutex The FreeRTOS semaphore handle
     * @param timeoutMs Timeout in milliseconds (0 = no wait, portMAX_DELAY = wait forever)
     */
    explicit MutexGuard(SemaphoreHandle_t mutex, uint32_t timeoutMs = 0) 
        : _mutex(mutex), _acquired(false) {
        if (!_mutex) {
            Serial.println("[MUTEX] Error: NULL mutex handle");
            return;
        }
        
        TickType_t timeout = (timeoutMs == 0) ? 0 : pdMS_TO_TICKS(timeoutMs);
        _acquired = (xSemaphoreTake(_mutex, timeout) == pdTRUE);
        
        if (!_acquired) {
            Serial.println("[MUTEX] Failed to acquire mutex (timeout)");
        }
    }
    
    /**
     * @brief Destructor - automatically release mutex if acquired
     */
    ~MutexGuard() {
        if (_acquired && _mutex) {
            xSemaphoreGive(_mutex);
        }
    }
    
    /**
     * @brief Check if mutex was successfully acquired
     * @return true if acquired, false otherwise
     */
    bool isAcquired() const { return _acquired; }
    
    // Delete copy operations - mutex guard should not be copied
    MutexGuard(const MutexGuard&) = delete;
    MutexGuard& operator=(const MutexGuard&) = delete;
    
private:
    SemaphoreHandle_t _mutex;
    bool _acquired;
};

/**
 * Handles audio playback via I2S on ESP32.
 * Supports tones, WAV files, and streaming audio.
 */
class Speaker {
public:
    // Audio configuration
    struct AudioConfig {
        uint32_t sampleRate;
        uint8_t bitsPerSample;
        uint8_t dmaBufCount;
        uint8_t dmaBufLen;
        
        AudioConfig() :
            sampleRate(::AudioConfig::SAMPLING_RATE),
            bitsPerSample(16),
            dmaBufCount(8),
            dmaBufLen(64)
        {}
    };

    // Callback type for playback state changes
    using PlaybackCallback = std::function<void(bool)>;

    /**
     * @brief Initialize and configure the speaker.
     * @param config Audio configuration settings.
     * @return true if successful, false otherwise.
     */
    bool begin(const AudioConfig& config = AudioConfig());
    
    // Audio playback methods
    /**
     * @brief Play a tone at specified frequency and duration.
     * @param frequency Tone frequency in Hz.
     * @param durationMs Duration in milliseconds.
     * @param volume Volume level (0.0 to 1.0).
     * @return true if successful, false otherwise.
     */
    bool playTone(float frequency, uint32_t durationMs, float volume = 1.0f);

    /**
     * @brief Play a WAV file from the file system.
     * @param path Path to the WAV file.
     * @param skipHeader Whether to skip the WAV header.
     * @return true if successful, false otherwise.
     */
    bool playWav(const char* path, bool skipHeader = true);

    /**
     * @brief Play audio from a buffer.
     * @param buffer Audio data buffer.
     * @param length Buffer length in bytes.
     * @return true if successful, false otherwise.
     */
    bool playBuffer(const uint8_t* buffer, size_t length);

    /**
     * @brief Play a chunk of audio data.
     * @param chunk Audio chunk data.
     * @param length Chunk length in bytes.
     * @return true if successful, false otherwise.
     */
    bool playChunk(const uint8_t* chunk, size_t length);
    
    // Utility methods
    /**
     * @brief Stop current audio playback.
     */
    void stop();

    /**
     * @brief Check if audio is currently playing.
     * @return true if playing, false otherwise.
     */
    bool isPlaying() const;

    /**
     * @brief Set callback for playback state changes.
     * @param callback Function called when playback starts/stops.
     */
    void setPlaybackCallback(PlaybackCallback callback) {
        _playbackCallback = callback;
    }
    
    // File system methods
    /**
     * @brief List files in a directory.
     * @param directory Directory path to list.
     */
    static void listFiles(const char* directory = "/");

private:
    // Hardware configuration
    static constexpr uint8_t BCLK_PIN = 1;  // Bit clock
    static constexpr uint8_t WS_PIN = 2;    // Word select
    static constexpr uint8_t DATA_PIN = 3;  // Data out
    
    // Internal state
    bool _initialized = false;
    bool _playing = false;
    AudioConfig _config;
    i2s_port_t _i2sPort = I2S_NUM_0;
    PlaybackCallback _playbackCallback;

    // Internal methods
    bool initI2S();
    bool initFileSystem();
    size_t writeSamples(const void* buffer, size_t bytes);
    void notifyPlaybackState(bool playing);
    void generateTone(float frequency, float volume, int16_t* buffer, size_t samples, uint32_t sampleRate);
    
};

#endif // SPEAKER_H
