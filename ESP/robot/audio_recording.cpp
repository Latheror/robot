#include "audio_recording.h"
#include "mqtt_handler.h"
#include <algorithm>

extern MqttHandler mqttHandler;

AudioRecording::AudioRecording(Indicators& indicators) : _indicators(indicators) {}

void AudioRecording::setRecordingFinishedCallback(std::function<void(const char* filePath)> callback) {
    _recordingFinishedCallback = callback;
}

void AudioRecording::startRecording() {
    _recordBuffer.clear();
    _recording = true;
    _lowSignalStart = 0;
    _indicators.set(Indicators::LED_PINS::IS_LISTENING, true);
    Serial.println("[AUDIO_REC] Recording started");
}

void AudioRecording::addSamples(const std::vector<int32_t>& samples) {
    if (_recording) {
        _recordBuffer.insert(_recordBuffer.end(), samples.begin(), samples.end());
    }
}

void AudioRecording::update(float currentVolume) {
    if (!_recording) return;

    unsigned long now = millis();

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
    file.write((const uint8_t*)"RIFF", 4);
    file.write((const uint8_t*)&fileSize, 4);
    file.write((const uint8_t*)"WAVE", 4);
    file.write((const uint8_t*)"fmt ", 4);

    uint32_t fmtSize = 16;
    file.write((const uint8_t*)&fmtSize, 4);

    uint16_t audioFormat = 1; // PCM
    file.write((const uint8_t*)&audioFormat, 2);
    file.write((const uint8_t*)&channels, 2);
    file.write((const uint8_t*)&sampleRate, 4);

    uint32_t byteRate = sampleRate * channels * (bitsPerSample / 8);
    file.write((const uint8_t*)&byteRate, 4);

    uint16_t blockAlign = channels * (bitsPerSample / 8);
    file.write((const uint8_t*)&blockAlign, 2);
    file.write((const uint8_t*)&bitsPerSample, 2);

    file.write((const uint8_t*)"data", 4);
    file.write((const uint8_t*)&dataSize, 4);

    // Write audio data (convert from 24-bit to 16-bit)
    for (int32_t sample : _recordBuffer) {
        // Sample is already 24-bit (shifted by 8), convert to 16-bit
        int16_t sample16 = (int16_t)(sample >> 8);
        file.write((const uint8_t*)&sample16, 2);
    }

    file.close();
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

    // Read entire file into buffer
    std::unique_ptr<uint8_t[]> fileBuffer(new uint8_t[fileSize]);
    if (!fileBuffer) {
        Serial.println("[AUDIO_REC] Failed to allocate buffer for file");
        file.close();
        return;
    }

    size_t bytesRead = file.read(fileBuffer.get(), fileSize);
    file.close();

    if (bytesRead != fileSize) {
        Serial.println("[AUDIO_REC] Failed to read complete file");
        return;
    }

    // Calculate base64 encoded size
    size_t base64Size = 0;
    mbedtls_base64_encode(nullptr, 0, &base64Size, fileBuffer.get(), fileSize);

    std::unique_ptr<char[]> base64Buffer(new char[base64Size]);
    if (!base64Buffer) {
        Serial.println("[AUDIO_REC] Failed to allocate base64 buffer");
        return;
    }

    if (mbedtls_base64_encode((unsigned char*)base64Buffer.get(), base64Size, &base64Size, fileBuffer.get(), fileSize) != 0) {
        Serial.println("[AUDIO_REC] Base64 encoding failed");
        return;
    }

    // Split into chunks and send via MQTT
    const size_t CHUNK_SIZE = 4096; // 4KB chunks
    int totalChunks = (base64Size + CHUNK_SIZE - 1) / CHUNK_SIZE; // Ceiling division

    Serial.printf("[AUDIO_REC] Sending %d bytes as base64 in %d chunks\n", (int)base64Size, totalChunks);

    for (int i = 0; i < totalChunks; i++) {
        size_t chunkStart = i * CHUNK_SIZE;
        size_t chunkSize = std::min(CHUNK_SIZE, base64Size - chunkStart);

        // Create JSON message using heap allocation to avoid stack overflow
        std::unique_ptr<char[]> message(new char[5120]);
        if (!message) {
            Serial.println("[AUDIO_REC] Failed to allocate message buffer");
            break;
        }
        
        snprintf(message.get(), 5120,
                 "{\"message\":\"%.*s\",\"chunk_index\":%d,\"total_chunks\":%d}",
                 (int)chunkSize, base64Buffer.get() + chunkStart, i, totalChunks);

        mqttHandler.publishMessage("robot/1/microphone", message.get());

        // Small delay between chunks to avoid overwhelming MQTT
        delay(10);
    }

    Serial.println("[AUDIO_REC] Audio transmission completed");

    // Clean up temporary file
    LittleFS.remove(filePath);
}