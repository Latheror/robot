#pragma once
#include <Arduino.h>
#include "oled_display.h"

/// <summary>
/// Encapsulates RoboEyes animation logic, tied to an OLED display.
/// Handles moods, animations, blinking, and timed updates.
/// </summary>
class RoboEyesDisplay {
public:
    /// <summary>
    /// Constructor. Requires an initialized OLEDDisplay instance.
    /// </summary>
    /// <param name="oled">Reference to an OLEDDisplay object.</param>
    RoboEyesDisplay(OLEDDisplay& oled);

    /// <summary>
    /// Initialize RoboEyes logic and random seed.
    /// </summary>
    void begin();

    /// <summary>
    /// Handle frame updates, OLED refresh, and mood/animation changes.
    /// Call this in the main loop.
    /// </summary>
    void update();

private:
    /// <summary>
    /// Checks if an I2C device is available at the given address.
    /// </summary>
    /// <param name="address">I2C device address</param>
    /// <returns>true if device responds, false otherwise</returns>
    bool isI2CAvailable(uint8_t address);

    OLEDDisplay& oled;

    /// <summary>
    /// Opaque pointer to RoboEyes object.
    /// </summary>
    void* roboEyesPtr;

    unsigned long lastFrame;
    unsigned long lastOledUpdate;
    unsigned long lastChange;

    static constexpr unsigned long frameInterval = 10;      // 100 FPS
    static constexpr unsigned long oledInterval = 100;     // 10 FPS
    static constexpr unsigned long changeInterval = 10000; // 10s
};
