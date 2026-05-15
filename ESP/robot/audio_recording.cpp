/**
 * @file audio_recording.cpp
 * @brief Implements bounded audio capture, WAV generation, and chunked MQTT upload.
 */

#include "audio_recording.h"
#include "mqtt_handler.h"
#include <algorithm>
#include <memory>
#include <new>

namespace {
constexpr size_t MQTT_RAW_AUDIO_CHUNK_SIZE = 3 * 1024;
constexpr size_t MQTT_BASE64_CHUNK_BUFFER_SIZE = ((MQTT_RAW_AUDIO_CHUNK_SIZE + 2) / 3) * 4 + 1;
constexpr size_t MQTT_JSON_MESSAGE_BUFFER_SIZE = MQTT_BASE64_CHUNK_BUFFER_SIZE + 128;

bool writeExact(File& file, const void* data, size_t length) {
    return file.write(static_cast<const uint8_t*>(data), length) == length;
}
}

extern MqttHandler mqttHandler;

AudioRecording::AudioRecording(Indicators& indicators, Speaker& speaker) : _indicators(indicators), _speaker(speaker) {}

void AudioRecording::setRecordingFinishedCallback(std::function<void(const char* filePath)> callback) {
    _recordingFinishedCallback = callback;
}

void AudioRecording::startRecording() {
    _recordBuffer.clear();
    _recordBuffer.reserve(MAX_RECORDING_SAMPLES);
    _recording = true;
    _lowSignalStart = 0;
    _indicators.set(Indicators::LED_PINS::IS_LISTENING, true);
    Serial.println("[AUDIO_REC] Recording started");
}

void AudioRecording::addSamples(const std::vector<int32_t>& samples) {
    if (!_recording || samples.empty()) {
        return;
    }

    const size_t remainingCapacity = MAX_RECORDING_SAMPLES - _recordBuffer.size();
    const size_t samplesToCopy = std::min(samples.size(), remainingCapacity);

    if (samplesToCopy > 0) {
        _recordBuffer.insert(_recordBuffer.end(), samples.begin(), samples.begin() + samplesToCopy);
    }

    if (_recordBuffer.size() >= MAX_RECORDING_SAMPLES) {
        Serial.println("[AUDIO_REC] Maximum recording duration reached, stopping recording");
        stopRecording();
    }
}

void AudioRecording::update(float currentVolume) {
    if (!_recording) return;

    unsigned long now = millis();

    // Periodically print sound level
    if (now - _lastPrintTime >= PRINT_INTERVAL_MS) {
        Serial.printf("[AUDIO_REC] Current sound level: %.4f\n", currentVolume);
        _lastPrintTime = now;
    }

    if (currentVolume < SILENCE_THRESHOLD) {
        if (_lowSignalStart == 0) {
            _lowSignalStart = now;
        } else if (now - _lowSignalStart >= SILENCE_TIMEOUT_MS) {
            Serial.println("[AUDIO_REC] Silence detected for 3 seconds, stopping recording");
            stopRecording();
        }
    } else {
        _lowSignalStart = 0; // Reset silence timer
    }
}

void AudioRecording::stopRecording() {
    if (!_recording) return;

    _recording = false;
    _indicators.set(Indicators::LED_PINS::IS_LISTENING, false);
    Serial.printf("[AUDIO_REC] Recording stopped, %d samples captured\n", (int)_recordBuffer.size());

    // Play "thinking" sound to indicate processing has started
    _speaker.playWav("/ok_im_thinking.wav");

    if (_recordBuffer.empty()) {
        Serial.println("[AUDIO_REC] No samples recorded");
        return;
    }

    if (createWavFile()) {
        if (_recordingFinishedCallback) {
            _recordingFinishedCallback(TEMP_RECORDING_FILE);
        }
        sendWavViaMQTT(TEMP_RECORDING_FILE);
    }
}

bool AudioRecording::createWavFile() {
    if (_recordBuffer.empty()) return false;

    const size_t maxSamplesForWav = (UINT32_MAX - SystemConfig::WAV_HEADER_SIZE) / sizeof(int16_t);
    if (_recordBuffer.size() > maxSamplesForWav) {
        Serial.println("[AUDIO_REC] Recording too large to encode as WAV");
        return false;
    }

    File file = LittleFS.open(TEMP_RECORDING_FILE, FILE_WRITE);
    if (!file) {
        Serial.println("[AUDIO_REC] Failed to create WAV file");
        return false;
    }

    // WAV file header (44 bytes)
    const uint32_t sampleRate = AudioConfig::SAMPLING_RATE;
    const uint16_t bitsPerSample = 16;
    const uint16_t channels = 1;
    const uint32_t dataSize = _recordBuffer.size() * (bitsPerSample / 8);
    const uint32_t fileSize = 44 + dataSize - 8; // Total file size minus "RIFF" + size

    // Write WAV header
    bool ok = true;
    ok = ok && writeExact(file, "RIFF", 4);
    ok = ok && writeExact(file, &fileSize, 4);
    ok = ok && writeExact(file, "WAVE", 4);
    ok = ok && writeExact(file, "fmt ", 4);

    uint32_t fmtSize = 16;
    ok = ok && writeExact(file, &fmtSize, 4);

    uint16_t audioFormat = 1; // PCM
    ok = ok && writeExact(file, &audioFormat, 2);
    ok = ok && writeExact(file, &channels, 2);
    ok = ok && writeExact(file, &sampleRate, 4);

    uint32_t byteRate = sampleRate * channels * (bitsPerSample / 8);
    ok = ok && writeExact(file, &byteRate, 4);

    uint16_t blockAlign = channels * (bitsPerSample / 8);
    ok = ok && writeExact(file, &blockAlign, 2);
    ok = ok && writeExact(file, &bitsPerSample, 2);

    ok = ok && writeExact(file, "data", 4);
    ok = ok && writeExact(file, &dataSize, 4);

    // Write audio data (convert from 24-bit to 16-bit)
    for (int32_t sample : _recordBuffer) {
        // Sample is already 24-bit (shifted by 8), convert to 16-bit
        int16_t sample16 = (int16_t)(sample >> 8);
        ok = ok && writeExact(file, &sample16, 2);
        if (!ok) {
            break;
        }
    }

    file.close();
    if (!ok) {
        Serial.println("[AUDIO_REC] Failed while writing WAV file");
        LittleFS.remove(TEMP_RECORDING_FILE);
        return false;
    }

    Serial.printf("[AUDIO_REC] WAV file created: %d samples, %d bytes\n", (int)_recordBuffer.size(), (int)(44 + dataSize));
    return true;
}

void AudioRecording::sendWavViaMQTT(const char* filePath) {
    File file = LittleFS.open(filePath, FILE_READ);
    if (!file) {
        Serial.println("[AUDIO_REC] Failed to open WAV file for MQTT transmission");
        return;
    }

    size_t fileSize = file.size();
    if (fileSize == 0) {
        Serial.println("[AUDIO_REC] WAV file is empty");
        file.close();
        return;
    }

    std::unique_ptr<uint8_t[]> rawBuffer(new (std::nothrow) uint8_t[MQTT_RAW_AUDIO_CHUNK_SIZE]);
    std::unique_ptr<char[]> base64Buffer(new (std::nothrow) char[MQTT_BASE64_CHUNK_BUFFER_SIZE]);
    std::unique_ptr<char[]> message(new (std::nothrow) char[MQTT_JSON_MESSAGE_BUFFER_SIZE]);

    if (!rawBuffer || !base64Buffer || !message) {
        Serial.println("[AUDIO_REC] Failed to allocate MQTT transmission buffers");
        file.close();
        LittleFS.remove(filePath);
        return;
    }

    const int totalChunks = static_cast<int>((fileSize + MQTT_RAW_AUDIO_CHUNK_SIZE - 1) / MQTT_RAW_AUDIO_CHUNK_SIZE);
    bool transmissionSuccess = true;

    Serial.printf("[AUDIO_REC] Sending %u bytes in %d MQTT chunks\n", static_cast<unsigned>(fileSize), totalChunks);

    for (int i = 0; i < totalChunks; i++) {
        const size_t bytesRead = file.read(rawBuffer.get(), MQTT_RAW_AUDIO_CHUNK_SIZE);
        if (bytesRead == 0) {
            Serial.printf("[AUDIO_REC] Failed to read audio chunk %d/%d from file\n", i + 1, totalChunks);
            transmissionSuccess = false;
            break;
        }

        size_t encodedLen = 0;
        if (mbedtls_base64_encode(
                reinterpret_cast<unsigned char*>(base64Buffer.get()),
                MQTT_BASE64_CHUNK_BUFFER_SIZE,
                &encodedLen,
                rawBuffer.get(),
                bytesRead) != 0) {
            Serial.printf("[AUDIO_REC] Base64 encoding failed for chunk %d/%d\n", i + 1, totalChunks);
            transmissionSuccess = false;
            break;
        }

        base64Buffer[encodedLen] = '\0';

        snprintf(message.get(), MQTT_JSON_MESSAGE_BUFFER_SIZE,
                 "{\"message\":\"%.*s\",\"chunk_index\":%d,\"total_chunks\":%d}",
                 static_cast<int>(encodedLen), base64Buffer.get(), i, totalChunks);

        if (!mqttHandler.publishMessage(MqttHandler::MQTT_TOPIC_MICROPHONE, message.get())) {
            Serial.printf("[AUDIO_REC] MQTT publish failed for chunk %d/%d\n", i + 1, totalChunks);
            transmissionSuccess = false;
            break;
        }

        // Small delay between chunks to avoid overwhelming MQTT
        delay(50);
    }

    file.close();

    if (transmissionSuccess) {
        Serial.println("[AUDIO_REC] Audio transmission completed");
    } else {
        Serial.println("[AUDIO_REC] Audio transmission aborted");
    }

    // Clean up temporary file
    LittleFS.remove(filePath);
}