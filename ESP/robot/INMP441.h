#ifndef INMP441_H
#define INMP441_H

#include <Arduino.h>
#include "driver/i2s.h"
#include "settings.h"

class INMP441 {
public:
    INMP441(int sampleRate = AUDIO_SAMPLING_RATE);

    bool begin();
    int32_t readSample();
    float getVolume();          // Returns the current volume level
    bool isVolumeAboveThreshold(); // Checks if volume is above threshold
    void update();             // Updates volume measurements

private:
    int _sampleRate;
    int32_t _samples[SOUND_AVERAGING_SAMPLES];
    int _sampleIndex = 0;
    float _currentVolume = 0;
    unsigned long _lastUpdate = 0;
    unsigned long _lastDebugPrint = 0;
    static const unsigned long UPDATE_INTERVAL = 50;   // 50ms between updates
    static const unsigned long DEBUG_INTERVAL = 1000;  // Print debug info every 1000ms

    // Pins for ESP32-S3
    static const int _pinBCLK = 37;  // BCLK / SCK
    static const int _pinLRCL = 36;  // LRCL / WS
    static const int _pinDOUT = 38;  // SD / Data

    i2s_port_t _i2sPort = I2S_NUM_1; // Use I2S0 port

    void calculateVolume();
};

#endif
