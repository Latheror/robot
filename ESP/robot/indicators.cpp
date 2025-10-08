#include "Indicators.h"
#include "PCF8575.h"

static PCF8575 pcf(0x20);

bool Indicators::begin() {
    Serial.println("[Indicators] Initializing...");

    if (!pcf.begin()) {
        Serial.println("[Indicators] PCF8575 initialization failed!");
        return false;
    }

    setAll(true);
    return true;
}

void Indicators::set(LED_PINS led, bool state) {
    Serial.printf("[Indicators] Setting LED %d to %s\n", led, state ? "ON" : "OFF");

    if (isValidLED(led)) {
        pcf.writePin(led, !state);
    }
}

void Indicators::setAll(bool state) {
    for (uint8_t i = 0; i < static_cast<uint8_t>(LED_PINS::COUNT); i++) {
        pcf.writePin(i, !state);
    }
}

void Indicators::toggle(LED_PINS led) {
    if (isValidLED(led)) {
        bool currentState = pcf.readPin(led);
        pcf.writePin(led, !currentState);
    }
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

void Indicators::flashSuccess() { blink(WIFI, 3, SHORT_FLASH); }
void Indicators::flashError()   { blink(WIFI, 1, LONG_FLASH);  }
void Indicators::flashWarning() { blink(WIFI, 2, MED_FLASH);   }

bool Indicators::isValidLED(LED_PINS led) const {
    return led < LED_PINS::COUNT;
}
