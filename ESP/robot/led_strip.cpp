#include "led_strip.h"

LEDStrip::LEDStrip(uint8_t pin, uint8_t numLeds)
    : _pin(pin), _numLeds(numLeds), _hue(0), _strip(numLeds, pin, NEO_GRB + NEO_KHZ800)
{
}

void LEDStrip::begin() {
    _strip.begin();
    _strip.show(); // Initialize all LEDs to off
    setBrightness(LED_BRIGHTNESS);
}

void LEDStrip::setColor(LedName led, uint32_t color) {
    if (led < _numLeds) _strip.setPixelColor(led, color);
}

void LEDStrip::setAll(uint32_t color) {
    for (uint8_t i = 0; i < _numLeds; i++) _strip.setPixelColor(i, color);
    _strip.show();
}

void LEDStrip::show() {
    _strip.show();
}

void LEDStrip::blink(uint32_t color, uint16_t delayMs) {
    setAll(color);
    delay(delayMs);
    setAll(0); // off
    delay(delayMs);
}

void LEDStrip::rainbow() {
    for (uint8_t i = 0; i < _numLeds; i++) {
        uint16_t hue = (_hue + i * 30) % 360;
        _strip.setPixelColor(i, _strip.ColorHSV(hue * 182)); // 0–65535
    }
    _strip.show();
    _hue = (_hue + 5) % 360;
    delay(20);
}

void LEDStrip::setBrightness(uint8_t brightness) {
    _strip.setBrightness(brightness);
    _strip.show();
}

uint32_t LEDStrip::color(uint8_t r, uint8_t g, uint8_t b) {
    return _strip.Color(r, g, b);
}