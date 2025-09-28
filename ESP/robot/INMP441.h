#ifndef INMP441_H
#define INMP441_H

#include <Arduino.h>
#include "driver/i2s.h"

class INMP441 {
public:
    INMP441(int sampleRate = 44100);

    bool begin();
    int32_t readSample();

private:
    int _sampleRate;

    // Pins pour ESP32-S3
    static const int _pinBCLK = 37;  // BCLK / SCK
    static const int _pinLRCL = 36;  // LRCL / WS
    static const int _pinDOUT = 38;  // SD / Data

    i2s_port_t _i2sPort = I2S_NUM_1; // Utiliser le port I2S0
};

#endif
