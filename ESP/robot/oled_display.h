#pragma once
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "settings.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

/**
 * @brief Manages the initialization and control of an SH1106 OLED display.
 * Provides I2C scanning and safe display startup.
 */
class OLEDDisplay {
public:
    /**
     * @brief Constructor for OLEDDisplay.
     */
    OLEDDisplay();

    /**
     * @brief Initialize the OLED display and perform an I2C scan.
     * @return true if successful, false otherwise.
     */
    bool begin();

    /**
     * @brief Clear the display contents.
     */
    void clear();

    /**
     * @brief Return a reference to the underlying Adafruit display driver.
     * @return Reference to the Adafruit_SH1106G display.
     */
    Adafruit_SH1106G& get();

    /**
     * @brief Check if the OLED display is initialized and available.
     * @return true if successfully initialized, false otherwise.
     */
    bool isInitialized() const;

private:
    /**
     * @brief Scan I2C bus and print detected devices.
     */
    void scanI2C();

    Adafruit_SH1106G display;
    bool initialized = false; ///< Tracks if the display was successfully initialized
};
