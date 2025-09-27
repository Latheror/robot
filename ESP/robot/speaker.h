#ifndef SPEAKER_H
#define SPEAKER_H

#include <Arduino.h>

class Speaker {
public:
    Speaker();
    void init();                     // Initialize pins and I2S
    void playTone(float frequency, int durationMs);
    void playExampleSound();         // Play a short demo melody

private:
    // Internal pin definitions
    static const int BCLK_PIN = 1;   // Adjust these pins to match your wiring
    static const int LRCK_PIN = 2;
    static const int DATA_PIN = 3;

    void i2sInit();
};

#endif
