#ifndef INDICATORS_H
#define INDICATORS_H

#include <Arduino.h>

class Indicators {
public:
    Indicators();
    void init();                  // Initialize the LED pins
    void setLED(int led, bool state);  // Turn a specific LED on/off
    void allOff();                // Turn all LEDs off
    void allOn();                 // Turn all LEDs on
    void blinkLED(int led, int times, int delayMs); // Blink LED

private:
    static const int LED1_PIN = 4;
    static const int LED2_PIN = 5;
    static const int LED3_PIN = 6;
};

#endif
