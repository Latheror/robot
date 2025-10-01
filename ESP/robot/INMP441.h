#ifndef INMP441_H
#define INMP441_H

#include <Arduino.h>
#include "driver/i2s.h"
#include "settings.h"

class INMP441 {
public:
    static constexpr int DEFAULT_SAMPLE_RATE = 44100;

    explicit INMP441(int sampleRate = DEFAULT_SAMPLE_RATE);
    
    bool begin();
    void update();
    
    int32_t readSample();
    float getVolume() const;
    bool isVolumeAboveThreshold() const;

private:
    const int _sampleRate;
    const i2s_port_t _i2sPort = I2S_NUM_1;

    float _currentVolume = 0.0f;
    
    unsigned long _lastUpdate = 0;
    unsigned long _lastDebugPrint = 0;

    static constexpr unsigned long UPDATE_INTERVAL = 20;   // ms
    static constexpr unsigned long DEBUG_INTERVAL  = 1000; // ms
    static constexpr double MAX_24BIT = 8388608.0;         // 2^23

    bool configureI2S() const;
};

#endif // INMP441_H
