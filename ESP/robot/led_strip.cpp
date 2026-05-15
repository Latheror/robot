/**
 * @file led_strip.cpp
 * @brief Implements bounded NeoPixel strip operations.
 */

#include "led_strip.h"

LEDStrip::LEDStrip(uint8_t pin, uint8_t numLeds)
    : _pin(pin), _numLeds(numLeds), _hue(0), _strip(numLeds, pin, NEO_GRB + NEO_KHZ800)
{
}

void LEDStrip::begin() {
    _strip.begin();
    _strip.show(); // Initialize all LEDs to off
    setBrightness(OledDisplayConfig::LED_BRIGHTNESS);
}

void LEDStrip::setColor(LedName led, uint32_t color) {
    if (led >= 0 && static_cast<uint8_t>(led) < _numLeds) {
        _strip.setPixelColor(static_cast<uint16_t>(led), color);
    }
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

void LEDStrip::setBrightness(uint8_t brightness) {
    _strip.setBrightness(brightness);
    _strip.show();
}

uint32_t LEDStrip::color(uint8_t r, uint8_t g, uint8_t b) {
    return _strip.Color(r, g, b);
}