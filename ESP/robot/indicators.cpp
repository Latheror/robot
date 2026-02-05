#include "indicators.h"

Indicators::Indicators(LEDStrip& strip) : _strip(strip) {}

bool Indicators::begin() {
    Serial.println("[Indicators] Initializing...");
    
    // The LED strip should already be initialized
    setAll(false);  // Turn off all LEDs initially
    return true;
}

void Indicators::set(LED_PINS led, bool state) {
    Serial.printf("[Indicators] Setting LED %d to %s\n", led, state ? "ON" : "OFF");

    if (!isValidLED(led)) return;
    _strip.setColor(static_cast<LedName>(led), state ? _strip.color(0, 255, 0) : 0);
    _strip.show();
}

void Indicators::setColor(LED_PINS led, uint8_t r, uint8_t g, uint8_t b) {
    Serial.printf("[Indicators] Setting LED %d to RGB(%d,%d,%d)\n", led, r, g, b);

    if (!isValidLED(led)) return;
    _strip.setColor(static_cast<LedName>(led), _strip.color(r, g, b));
    _strip.show();
}

void Indicators::setAll(bool state) {
    uint32_t color = state ? _strip.color(0, 255, 0) : 0;
    _strip.setAll(color);
    _strip.show();
}

void Indicators::toggle(LED_PINS led) {
    if (!isValidLED(led)) return;
    // Since we can't read the current state from the LED strip,
    // we'll just toggle between green and off
    static bool states[COUNT] = {false};
    states[led] = !states[led];
    set(led, states[led]);
}

void Indicators::blink(LED_PINS led, uint8_t times, uint16_t delayMs) {
    Serial.printf("[Indicators] Blinking LED %d, %d times, %d ms delay\n", led, times, delayMs);

    if (!isValidLED(led)) return;

    for (uint8_t i = 0; i < times; i++) {
        set(led, true);
        delay(delayMs);
        set(led, false);
        if (i < times - 1) delay(delayMs);
    }
}

bool Indicators::isValidLED(LED_PINS led) const {
    return led < COUNT;
}
