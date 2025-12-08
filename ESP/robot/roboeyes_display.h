#pragma once
#include <Arduino.h>
#include "oled_display.h"

/**
 * @brief Encapsulates RoboEyes animation logic, tied to an OLED display.
 * Handles moods, animations, blinking, and timed updates.
 */
class RoboEyesDisplay {
public:
    /**
     * @brief Constructor. Requires an initialized OLEDDisplay instance.
     * @param oled Reference to an OLEDDisplay object.
     */
    RoboEyesDisplay(OLEDDisplay& oled);

    /**
     * @brief Initialize RoboEyes logic and random seed.
     */
    void begin();

    /**
     * @brief Handle frame updates, OLED refresh, and mood/animation changes.
     * Call this in the main loop.
     */
    void update();

private:
    OLEDDisplay& oled; ///< Reference to the OLED display

    /**
     * @brief Opaque pointer to RoboEyes object.
     */
    void* roboEyesPtr;

    unsigned long lastFrame;     ///< Last frame update time
    unsigned long lastOledUpdate; ///< Last OLED update time
    unsigned long lastChange;     ///< Last mood change time
    bool oledAvailable;           ///< Tracks if OLED is available

    static constexpr unsigned long frameInterval = 10;      ///< 100 FPS
    static constexpr unsigned long oledInterval = 100;     ///< 10 FPS
    static constexpr unsigned long changeInterval = 10000; ///< 10s
};
