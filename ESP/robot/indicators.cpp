#include "Indicators.h"

bool Indicators::begin() {
    // Configure all LED pins as outputs
    for (size_t i = 0; i < static_cast<size_t>(LED::COUNT); i++) {
        pinMode(PIN_MAP[i], OUTPUT);
    }
    
    // Initialize all LEDs to off state
    setAll(false);
    return true;
}

void Indicators::set(LED led, bool state) {
    if (isValidLED(led)) {
        digitalWrite(getLEDPin(led), state ? HIGH : LOW);
    }
}

void Indicators::setAll(bool state) {
    for (size_t i = 0; i < static_cast<size_t>(LED::COUNT); i++) {
        digitalWrite(PIN_MAP[i], state ? HIGH : LOW);
    }
}

void Indicators::toggle(LED led) {
    if (isValidLED(led)) {
        uint8_t pin = getLEDPin(led);
        digitalWrite(pin, !digitalRead(pin));
    }
}

void Indicators::blink(LED led, uint8_t times, uint16_t delayMs) {
    if (!isValidLED(led)) return;
    
    for (uint8_t i = 0; i < times; i++) {
        set(led, true);
        delay(delayMs);
        set(led, false);
        if (i < times - 1) {  // Don't delay after last blink
            delay(delayMs);
        }
    }
}

void Indicators::flashSuccess() {
    blink(LED::STATUS, 3, SHORT_FLASH);
}

void Indicators::flashError() {
    blink(LED::STATUS, 1, LONG_FLASH);
}

void Indicators::flashWarning() {
    blink(LED::STATUS, 2, MED_FLASH);
}

bool Indicators::isValidLED(LED led) const {
    return static_cast<size_t>(led) < static_cast<size_t>(LED::COUNT);
}

uint8_t Indicators::getLEDPin(LED led) const {
    return PIN_MAP[static_cast<size_t>(led)];
}
