#pragma once
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SDA_PIN 17
#define SCL_PIN 18
#define OLED_ADDR 0x3C

/// <summary>
/// Manages the initialization and control of an SH1106 OLED display.
/// Provides I2C scanning and safe display startup.
/// </summary>
class OLEDDisplay {
public:
    /// <summary>
    /// Constructor for OLEDDisplay.
    /// </summary>
    OLEDDisplay();

    /// <summary>
    /// Initialize the OLED display and perform an I2C scan.
    /// </summary>
    void begin();

    /// <summary>
    /// Clear the display contents.
    /// </summary>
    void clear();

    /// <summary>
    /// Return a reference to the underlying Adafruit display driver.
    /// </summary>
    Adafruit_SH1106G& get();

private:
    /// <summary>
    /// Scan I2C bus and print detected devices.
    /// </summary>
    void scanI2C();

    Adafruit_SH1106G display;
};
