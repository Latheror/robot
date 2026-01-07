#include "oled_display.h"

/// <summary>
/// Constructor for OLEDDisplay.
/// Initializes the Adafruit_SH1106G member.
/// </summary>
OLEDDisplay::OLEDDisplay() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1) {}

/// <summary>
/// Perform I2C scan and print all connected devices.
/// </summary>
void OLEDDisplay::scanI2C() {
    byte error, address;
    int nDevices = 0;

    Serial.println("Scanning I2C bus...");

    for (address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0) {
            Serial.print("I2C device found at address 0x");
            if (address < 16) Serial.print("0");
            Serial.print(address, HEX);
            Serial.println(" !");
            nDevices++;
        }
    }

    if (nDevices == 0) {
        Serial.println("No I2C devices found\n");
    } else {
        Serial.println("I2C scan complete\n");
    }
}

/// <summary>
/// Initialize the OLED display and perform an I2C scan.
/// </summary>
bool OLEDDisplay::begin() {
    Wire.begin(PinConfig::DISPLAY_SDA, PinConfig::DISPLAY_SCL, 100000); // SDA, SCL, frequency
    delay(100);

    scanI2C(); // scan before initializing the display

    if (!display.begin(OLED_ADDR, true)) {
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
