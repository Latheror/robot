/**
 * @file oled_display.cpp
 * @brief Implements safe SH1106 OLED initialization and access helpers.
 */

#include "oled_display.h"

OLEDDisplay::OLEDDisplay() : display(OledDisplayConfig::SCREEN_WIDTH, OledDisplayConfig::SCREEN_HEIGHT, &Wire, -1) {}

bool OLEDDisplay::begin() {
    delay(SystemConfig::OLED_INIT_DELAY_MS);

    if (!display.begin(OledDisplayConfig::OLED_ADDR, true)) {
        Serial.println(F("[OLED] Failed to initialize SH1106"));
        initialized = false;
        return false; // Don't halt, just return false
    }

    display.clearDisplay();
    display.display();
    initialized = true;
    return true;
}

void OLEDDisplay::clear() {
    if (!initialized) {
        return;
    }

    display.clearDisplay();
    display.display();
}

Adafruit_SH1106G& OLEDDisplay::get() {
    return display;
}

bool OLEDDisplay::isInitialized() const {
    return initialized;
}
