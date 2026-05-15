/**
 * @file audio_recording.h
 * @brief Records microphone samples, writes temporary WAV files, and publishes them over MQTT.
 */

#ifndef AUDIO_RECORDING_H
#define AUDIO_RECORDING_H

#include <Arduino.h>
#include <vector>
#include <functional>
#include <FS.h>
#include <LittleFS.h>
#include <mbedtls/base64.h>
#include "settings.h"
#include "indicators.h"
#include "speaker.h"

/**
 * @class AudioRecording
 * @brief Handles audio recording, WAV file creation, and MQTT transmission.
 *
 * This class manages:
 * - Automatic recording start/stop based on audio levels
 * - WAV file generation from raw samples
 * - Base64 encoding and chunked MQTT transmission
 */
class AudioRecording {
public:
    /**
     * @brief Constructor.
     * @param indicators Reference to the indicators system
     * @param speaker Reference to the speaker system
     */
    AudioRecording(Indicators& indicators, Speaker& speaker);

    /**
     * @brief Set callback for when recording is finished and ready to send.
     * @param callback Function called when recording completes with WAV file ready
     */
    void setRecordingFinishedCallback(std::function<void(const char* filePath)> callback);

    /**
     * @brief Start recording audio.
     * @param samples Vector of int32_t audio samples
     */
    void startRecording();

    /**
     * @brief Add samples to the recording buffer.
     * @param samples Vector of int32_t audio samples
     */
    void addSamples(const std::vector<int32_t>& samples);

    /**
     * @brief Update recording state - check for auto-stop conditions.
     * @param currentVolume Current audio volume level (0.0 - 1.0)
     */
    void update(float currentVolume);

    /**
     * @brief Stop recording and process the audio.
     */
    void stopRecording();

    /**
     * @brief Check if currently recording.
     * @return true if recording, false otherwise
     */
    bool isRecording() const { return _recording; }

    /**
     * @brief Get the current recording buffer.
     * @return Vector of recorded samples
     */
    const std::vector<int32_t>& getBuffer() const { return _recordBuffer; }

private:
    bool _recording = false;                    ///< Recording state flag
    std::vector<int32_t> _recordBuffer;         ///< Audio sample buffer
    unsigned long _lowSignalStart = 0;          ///< Timestamp when signal became low
    unsigned long _lastPrintTime = 0;           ///< Last time sound level was printed
    std::function<void(const char* filePath)> _recordingFinishedCallback; ///< Callback when recording finishes
    Indicators& _indicators;                    ///< Reference to indicators system
    Speaker& _speaker;                          ///< Reference to speaker system

    static constexpr unsigned long SILENCE_TIMEOUT_MS = 3000; ///< 3 seconds of low signal to stop
    static constexpr float SILENCE_THRESHOLD = 0.003f;        ///< Volume threshold for silence detection (much lower than clap threshold)
    static constexpr unsigned long PRINT_INTERVAL_MS = 500;   ///< Print sound level every 500ms
    static constexpr size_t MAX_RECORDING_SAMPLES = AudioConfig::SAMPLING_RATE * 12; ///< Safety cap: 12 seconds at 16 kHz
    static constexpr const char* TEMP_RECORDING_FILE = "/temp_recording.wav"; ///< Temporary WAV file path

    /**
     * @brief Create WAV file from recorded samples.
     * @return true if successful, false otherwise
     */
    bool createWavFile();

    /**
     * @brief Send WAV file via MQTT in base64 chunks.
     * @param filePath Path to the WAV file
     */
    void sendWavViaMQTT(const char* filePath);
};

#endif // AUDIO_RECORDING_H