#ifndef SPEAKER_H
#define SPEAKER_H

#include <Arduino.h>

/// <summary>
/// Speaker class handles audio playback via I2S on ESP32,
/// including tones, WAV files from LittleFS, and streaming buffers.
/// </summary>
class Speaker {
public:
    /// <summary>
    /// Constructor
    /// </summary>
    Speaker();

    /// <summary>
    /// Initialize I2S and mount LittleFS
    /// </summary>
    void init();

    /// <summary>
    /// Play a sine wave tone
    /// </summary>
    /// <param name="frequency">Frequency in Hz</param>
    /// <param name="durationMs">Duration in milliseconds</param>
    /// <param name="volume">Volume (0.0 to 1.0)</param>
    void playTone(float frequency, int durationMs, float volume);

    /// <summary>
    /// Play a short example melody
    /// </summary>
    void playExampleSound();

    /// <summary>
    /// Play a WAV file from LittleFS
    /// </summary>
    /// <param name="path">Path to the WAV file</param>
    void playWav(const char* path);

    /// <summary>
    /// List files in LittleFS
    /// </summary>
    void listFiles();

    /// <summary>
    /// Play a buffer containing WAV audio
    /// </summary>
    /// <param name="buffer">Pointer to audio buffer</param>
    /// <param name="len">Buffer length in bytes</param>
    void playWavFromBuffer(uint8_t* buffer, size_t len);

    /// <summary>
    /// Play a chunk of WAV audio via I2S.
    /// Useful for streaming audio in smaller pieces.
    /// </summary>
    /// <param name="buffer">Pointer to chunk buffer</param>
    /// <param name="len">Length of chunk in bytes</param>
    void playWavChunk(const uint8_t* buffer, size_t len);

private:
    // Internal pin definitions
    static const int BCLK_PIN = 1;   // Bit clock pin
    static const int LRCK_PIN = 2;   // Word select / left-right clock pin
    static const int DATA_PIN = 3;   // Data pin

    /// <summary>
    /// Initialize I2S driver with pins and configuration
    /// </summary>
    void i2sInit();
};

#endif
