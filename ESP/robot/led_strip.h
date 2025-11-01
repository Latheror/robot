#pragma once
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define LED_STRIP_PIN 13
#define STRIP_LED_COUNT 5
#define LED_BRIGHTNESS 64

enum LedName {
    LED_NAME_1 = 0,
    LED_NAME_2,
    LED_NAME_3,
    LED_NAME_4,
    LED_NAME_5
};

class LEDStrip {
public:
    LEDStrip(uint8_t pin = LED_STRIP_PIN, uint8_t numLeds = STRIP_LED_COUNT);
    void begin();

    void setColor(LedName led, uint32_t color);
    void setAll(uint32_t color);
    void show();
    void blink(uint32_t color, uint16_t delayMs);
    void rainbow();
    void setBrightness(uint8_t brightness);
    uint32_t color(uint8_t r, uint8_t g, uint8_t b);

private:
    uint8_t _pin;
    uint8_t _numLeds;
    uint8_t _hue;
    Adafruit_NeoPixel _strip;
};
