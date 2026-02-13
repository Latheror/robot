#ifndef ROBOEYES_DISPLAY_H
#define ROBOEYES_DISPLAY_H

#include <Arduino.h>
#include "oled_display.h"

/**
 * @brief Enumeration for RoboEyes moods.
 */
enum class Mood {
    MOOD_HAPPY,
    MOOD_TIRED,
    MOOD_ANGRY,
    MOOD_DEFAULT
};

/**
 * @brief Enumeration for RoboEyes positions.
 */
enum class Position {
    POS_N, POS_NE, POS_E, POS_SE, POS_S, POS_SW, POS_W, POS_NW, POS_DEFAULT
};

/**
 * @brief Enumeration for RoboEyes animations.
 */
enum class Animation {
    ANIM_BLINK,
    ANIM_LAUGH,
    ANIM_CONFUSED
};

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

    /**
     * @brief Set the mood of the RoboEyes.
     * @param mood The mood to set.
     * @return True if the mood was set successfully, false otherwise.
     */
    bool setMood(Mood mood);

    /**
     * @brief Trigger an animation on the RoboEyes.
     * @param animation The animation to trigger.
     * @return True if the animation was triggered successfully, false otherwise.
     */
    bool triggerAnimation(Animation animation);

    /**
     * @brief Enable MQTT control mode (stops automatic random changes).
     * Called when the first MQTT message is received.
     */
    void enableMqttControl();

    /**
     * @brief Set the position of the RoboEyes.
     * @param position The position to set.
     * @return True if the position was set successfully, false otherwise.
     */
    bool setPosition(Position position);

    /**
     * @brief Set curiosity mode.
     * @param enabled Whether curiosity mode is enabled.
     * @return True if the setting was applied successfully, false otherwise.
     */
    bool setCuriosity(bool enabled);

    /**
     * @brief Set sweat mode.
     * @param enabled Whether sweat mode is enabled.
     * @return True if the setting was applied successfully, false otherwise.
     */
    bool setSweat(bool enabled);

    /**
     * @brief Set horizontal flicker.
     * @param enabled Whether horizontal flicker is enabled.
     * @param amplitude The amplitude of the flicker in pixels.
     * @return True if the setting was applied successfully, false otherwise.
     */
    bool setHFlicker(bool enabled, uint8_t amplitude = 2);

    /**
     * @brief Set vertical flicker.
     * @param enabled Whether vertical flicker is enabled.
     * @param amplitude The amplitude of the flicker in pixels.
     * @return True if the setting was applied successfully, false otherwise.
     */
    bool setVFlicker(bool enabled, uint8_t amplitude = 2);

    /**
     * @brief Set autoblinker.
     * @param enabled Whether autoblinker is enabled.
     * @param interval Interval between blinks in seconds.
     * @param variation Random variation in seconds.
     * @return True if the setting was applied successfully, false otherwise.
     */
    bool setAutoblinker(bool enabled, int interval = 3, int variation = 2);

    /**
     * @brief Set idle mode.
     * @param enabled Whether idle mode is enabled.
     * @param interval Interval between repositioning in seconds.
     * @param variation Random variation in seconds.
     * @return True if the setting was applied successfully, false otherwise.
     */
    bool setIdleMode(bool enabled, int interval = 2, int variation = 2);

    /**
     * @brief Open the eyes.
     * @param left Whether to open the left eye.
     * @param right Whether to open the right eye.
     * @return True if the operation was successful, false otherwise.
     */
    bool openEyes(bool left = true, bool right = true);

    /**
     * @brief Close the eyes.
     * @param left Whether to close the left eye.
     * @param right Whether to close the right eye.
     * @return True if the operation was successful, false otherwise.
     */
    bool closeEyes(bool left = true, bool right = true);

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
    bool mqttControlled;          ///< Tracks if face has been controlled via MQTT

    static constexpr unsigned long frameInterval = 10;      ///< 100 FPS
    static constexpr unsigned long oledInterval = 100;     ///< 10 FPS
    static constexpr unsigned long changeInterval = 10000; ///< 10s
};

#endif // ROBOEYES_DISPLAY_H
