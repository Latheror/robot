#ifndef INMP441_H
#define INMP441_H

#include <Arduino.h>
#include "driver/i2s.h"
#include "settings.h"
#include <functional>

class INMP441 {
public:
    static constexpr int DEFAULT_SAMPLE_RATE = 24000;  // Match speaker sample rate

    explicit INMP441(int sampleRate = DEFAULT_SAMPLE_RATE);
    
    bool begin();
    void update();
    
    int32_t readSample();
    float getVolume() const;
    bool isVolumeAboveThreshold() const;

    // Clap detection
    void setClapCallback(std::function<void()> callback);

private:
    const int _sampleRate;
    const i2s_port_t _i2sPort = I2S_NUM_1;

    float _currentVolume = 0.0f;
    
    unsigned long _lastUpdate = 0;
    unsigned long _lastDebugPrint = 0;

    static constexpr unsigned long UPDATE_INTERVAL = 20;   // ms
    static constexpr unsigned long DEBUG_INTERVAL  = 1000; // ms
    static constexpr double MAX_24BIT = 8388607.0;         // 2^23 - 1
    static constexpr double CLAP_THRESHOLD = 0.6;          // Volume threshold for clap (normalized 0.0-1.0)
    static constexpr unsigned long CLAP_DEBOUNCE = 300;    // ms to ignore multiple detections

    unsigned long _lastClapTime = 0;
    std::function<void()> _clapCallback;

    bool configureI2S() const;
    void checkForClap();
};

#endif // INMP441_H
