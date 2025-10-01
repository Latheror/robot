#ifndef INMP441_H
#define INMP441_H

#include <Arduino.h>
#include "driver/i2s.h"
#include "settings.h"

class INMP441 {
public:
    // Microphone configuration
    static constexpr int DEFAULT_SAMPLE_RATE = 44100;
    static constexpr int AVERAGING_SAMPLES = 64;

    // Constructor with default sample rate
    explicit INMP441(int sampleRate = DEFAULT_SAMPLE_RATE);
    
    // Initialize the microphone
    bool begin();
    
    // Update volume measurements and LED status
    void update();
    
    // Get current audio metrics
    int32_t readSample();                // Read raw audio sample
    float getVolume() const;             // Get current volume level
    bool isVolumeAboveThreshold() const; // Check if volume exceeds threshold

private:
    // Configuration
    const int _sampleRate;
    const i2s_port_t _i2sPort = I2S_NUM_1;
    
    // Audio processing
    int32_t _samples[AVERAGING_SAMPLES] = {0};
    int _sampleIndex = 0;
    float _currentVolume = 0.0f;
    
    // Timing
    unsigned long _lastUpdate = 0;
    unsigned long _lastDebugPrint = 0;
    static constexpr unsigned long UPDATE_INTERVAL = 50;    // ms
    static constexpr unsigned long DEBUG_INTERVAL = 1000;   // ms
    
    // Internal methods
    void calculateVolume();
    bool configureI2S() const;
};

#endif // INMP441_H
