#include "Indicators.h"
#include "PCF8575.h"

// Create a global (or static) instance of PCF8575
// You can change the address (0x20–0x27) to match your hardware configuration
static PCF8575 pcf(0x20);

bool Indicators::begin() {

    Serial.println("[Indicators] Initializing...");

    // Initialize the PCF8575 device
    if (!pcf.begin()) {
        Serial.println("[Indicators] PCF8575 initialization failed!");
        return false;
    }

    // Initialize all LEDs to off state
    //setAll(false);
    setAll(true);
    return true;
}

void Indicators::set(LED led, bool state) {
    if (isValidLED(led)) {
        uint8_t pin = getLEDPin(led);
        // Using PCF8575 instead of direct GPIO
        pcf.writePin(pin, state);
    }
}

void Indicators::setAll(bool state) {
    // Loop over all LEDs and set each
    for (size_t i = 0; i < static_cast<size_t>(LED::COUNT); i++) {
        pcf.writePin(PIN_MAP[i], state);
    }
}

void Indicators::toggle(LED led) {
    if (isValidLED(led)) {
        uint8_t pin = getLEDPin(led);
        bool currentState = pcf.readPin(pin);
        pcf.writePin(pin, !currentState);
    }
}

void Indicators::blink(LED led, uint8_t times, uint16_t delayMs) {
    if (!isValidLED(led)) return;

    for (uint8_t i = 0; i < times; i++) {
        set(led, true);
        delay(delayMs);
        set(led, false);
        if (i < times - 1) {
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
