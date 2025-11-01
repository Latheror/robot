#ifndef RGB_LED_H
#define RGB_LED_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

class RGBLed {
public:
    static constexpr uint8_t RGB_LED_PIN = 48;  // RGB LED pin number
    static constexpr uint8_t SINGLE_LED_COUNT = 1; // Number of LEDs (always 1 for this class)
    
    RGBLed() : _pixel(SINGLE_LED_COUNT, RGB_LED_PIN, NEO_GRB + NEO_KHZ800) {}
    
    bool begin() {
        _pixel.begin();
        _pixel.clear();
        _pixel.show();
        return true;
    }
    
    void setColor(uint8_t r, uint8_t g, uint8_t b) {
        _pixel.setPixelColor(0, _pixel.Color(r, g, b));
        _pixel.show();
    }
    
    void clear() {
        _pixel.clear();
        _pixel.show();
    }
    
    void setBrightness(uint8_t brightness) {
        _pixel.setBrightness(brightness);
        _pixel.show();
    }

private:
    Adafruit_NeoPixel _pixel;
};

#endif // RGB_LED_H
