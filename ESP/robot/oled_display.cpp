#include "oled_display.h"

/// <summary>
/// Constructor for OLEDDisplay.
/// Initializes the Adafruit_SH1106G member.
/// </summary>
OLEDDisplay::OLEDDisplay() : display(OledDisplayConfig::SCREEN_WIDTH, OledDisplayConfig::SCREEN_HEIGHT, &Wire, -1) {}

/// <summary>
/// Initialize the OLED display.
/// </summary>
bool OLEDDisplay::begin() {
    delay(100);

    if (!display.begin(OledDisplayConfig::OLED_ADDR, true)) {
        Serial.println(F("Failed to initialize SH1106"));
        initialized = false;
        return false; // Don't halt, just return false
    }

    display.clearDisplay();
    display.display();
    initialized = true;
    return true;
}

/// <summary>
/// Clear the OLED display.
/// </summary>
void OLEDDisplay::clear() {
    display.clearDisplay();
    display.display();
}

/// <summary>
/// Return a reference to the internal Adafruit display object.
/// </summary>
Adafruit_SH1106G& OLEDDisplay::get() {
    return display;
}

/// <summary>
/// Check if the OLED display is initialized and available.
/// </summary>
bool OLEDDisplay::isInitialized() const {
    return initialized;
}
