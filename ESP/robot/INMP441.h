/**
 * @file INMP441.h
 * @brief I2S microphone driver with RMS volume tracking and double-clap detection.
 */

#ifndef INMP441_H
#define INMP441_H


#include <Arduino.h>
#include "driver/i2s.h"
#include "settings.h"
#include <functional>
#include <vector>

/**
 * @class INMP441
 * @brief Driver for the I2S microphone INMP441 with double-clap detection and voice recording.
 *
 * This class manages:
 * - I2S configuration for the microphone.
 * - RMS volume computation.
 * - Voice activity LED indication.
 * - Double-clap detection and callback.
 * - Voice recording buffer capture.
 */
class INMP441 {
public:
    /**
     * @brief Default sample rate (must match speaker rate).
     */
    static constexpr int DEFAULT_SAMPLE_RATE = ::AudioConfig::SAMPLING_RATE;

    /**
     * @brief Constructor.
     * @param sampleRate Desired sample rate (default: 16000 Hz)
     */
    explicit INMP441(int sampleRate = DEFAULT_SAMPLE_RATE);

    /**
     * @brief Initialize microphone and configure I2S.
     * @return true if successful, false otherwise
     */
    bool begin();

    /**
     * @brief Update audio samples, compute volume, and detect claps.
     * Call this regularly (~20 ms) for responsive detection.
     */
    void update();

    /**
     * @brief Read multiple samples for recording purposes.
     * @param buffer Buffer to store samples
     * @param numSamples Number of samples to read
     * @return Number of samples actually read
     */
    int readSamplesForRecording(int32_t* buffer, int numSamples);

    /**
     * @brief Get current RMS volume.
     * @return Volume (0.0 - 1.0)
     */
    float getVolume() const;

    /**
     * @brief Check if volume exceeds threshold.
     * @return true if volume above threshold, false otherwise
     */
    bool isVolumeAboveThreshold() const;

    /**
     * @brief Set callback for double-clap detection.
     * @param callback Function called on double clap
     */
    void setClapCallback(std::function<void()> callback);

    /**
     * @brief Set callback to check if recording is active.
     * @param callback Function returning true if recording, false otherwise
     */
    void setIsRecordingCallback(std::function<void(bool)> callback);

    /**
     * @brief Enable clap detection
     */
    void enableClapDetection() { _clapDetectionEnabled = true; }

    /**
     * @brief Disable clap detection
     */
    void disableClapDetection() { _clapDetectionEnabled = false; }

    /**
     * @brief Check if clap detection is enabled
     * @return true if enabled, false otherwise
     */
    bool isClapDetectionEnabled() const { return _clapDetectionEnabled; }

private:
    const int _sampleRate;                  ///< Configured sample rate
    const i2s_port_t _i2sPort = I2S_NUM_1; ///< I2S port

    float _currentVolume = 0.0f;            ///< Current RMS volume

    unsigned long _lastUpdate = 0;          ///< Last update timestamp
    unsigned long _lastClapTime = 0;        ///< Last clap timestamp
    unsigned long _firstClapTime = 0;       ///< First clap timestamp for double detection

    std::function<void()> _clapCallback;    ///< Double-clap callback
    std::function<void(bool)> _isRecordingCallback; ///< Reserved recording-state callback for API compatibility

    bool _clapDetectionEnabled = true;      ///< Clap detection enabled flag

    static constexpr unsigned long UPDATE_INTERVAL = 20;  
    static constexpr double MAX_24BIT = 8388607.0;
    static constexpr double CLAP_THRESHOLD = 0.5;
    static constexpr unsigned long CLAP_DEBOUNCE = 300;
    static constexpr unsigned long DOUBLE_CLAP_MAX_DELAY = 500;

    /**
     * @brief Configure I2S interface
     * @return true if successful, false otherwise
     */
    bool configureI2S() const;

    /**
     * @brief Read microphone samples and compute RMS volume
     */
    void readSamplesAndComputeVolume();

    /**
     * @brief Update the voice activity LED
     */
    void updateActivityLed();

    /**
     * @brief Detect double clap based on RMS volume
     */
    void detectDoubleClap();
};

#endif // INMP441_H
