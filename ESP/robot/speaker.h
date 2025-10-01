#ifndef SPEAKER_H
#define SPEAKER_H

#include <Arduino.h>
#include <driver/i2s.h>
#include "FS.h"

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
            sampleRate(24000),
            bitsPerSample(16),
            dmaBufCount(8),
            dmaBufLen(64)
        {}
    };

    // Initialize and configure the speaker
    bool begin(const AudioConfig& config = AudioConfig());
    
    // Audio playback methods
    bool playTone(float frequency, uint32_t durationMs, float volume = 1.0f);
    bool playWav(const char* path, bool skipHeader = true);
    bool playBuffer(const uint8_t* buffer, size_t length);
    bool playChunk(const uint8_t* chunk, size_t length);
    
    // Utility methods
    void stop();
    bool isPlaying() const;
    
    // Example methods
    void playExampleSound() {
        playTone(440, 500);  // Play A4 note for 500ms
    }
    
    // File system methods
    static bool checkFile(const char* path);
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

    // Internal methods
    bool initI2S();
    bool initFileSystem();
    size_t writeSamples(const void* buffer, size_t bytes);
    
    // Tone generation
    static void generateTone(float frequency, float volume, 
                           int16_t* buffer, size_t samples,
                           uint32_t sampleRate);
};

#endif // SPEAKER_H
