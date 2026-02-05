#ifndef INDICATORS_H
#define INDICATORS_H

#include <Arduino.h>
#include "led_strip.h"

/**
 * @class Indicators
 * @brief High-level interface for managing system LED indicators through LED strip.
 *
 * This class provides control over several indicator LEDs such as status, network,
 * and activity. Each LED can be turned on/off, toggled, or blinked with custom patterns.
 */
class Indicators {
public:
    /**
     * @enum LED_PINS
     * @brief Defines LED identifiers and their corresponding LED strip positions.
     */
    enum LED_PINS : uint8_t {
        WIFI           = LED_NAME_1,  ///< WiFi status indicator
        MQTT          = LED_NAME_2,  ///< MQTT connection indicator
        IS_LISTENING  = LED_NAME_3,  ///< Currently listening indicator
        IS_SPEAKING   = LED_NAME_4,  ///< Currently speaking indicator
        MOTORS_MOVING = LED_NAME_5,  ///< Robot arm movement indicator
        COUNT = STRIP_LED_COUNT      ///< Number of available LEDs
    };

    /**
     * @brief Constructor that takes a reference to the LED strip.
     * 
     * @param strip Reference to the LED strip object.
     */
    Indicators(LEDStrip& strip);

    /**
     * @brief Initializes the indicator system.
     * 
     * @return true if initialization was successful, false otherwise.
     */
    bool begin();

    /**
     * @brief Sets the state of a specific LED.
     * 
     * @param led The LED to modify (from LED_PINS enum).
     * @param state True to turn the LED on (green), false to turn it off.
     */
    void set(LED_PINS led, bool state);

    /**
     * @brief Sets a specific LED to a custom color.
     * 
     * @param led The LED to modify (from LED_PINS enum).
     * @param r Red component (0-255).
     * @param g Green component (0-255).
     * @param b Blue component (0-255).
     */
    void setColor(LED_PINS led, uint8_t r, uint8_t g, uint8_t b);

    /**
     * @brief Sets the state of all LEDs at once.
     * 
     * @param state True to turn all LEDs on (green), false to turn them off.
     */
    void setAll(bool state);

    /**
     * @brief Toggles the state of a specific LED.
     * 
     * @param led The LED to toggle.
     */
    void toggle(LED_PINS led);

    /**
     * @brief Blinks a specific LED a given number of times.
     * 
     * @param led The LED to blink.
     * @param times Number of blink repetitions (default is 1).
     * @param delayMs Delay between on/off states in milliseconds (default is 100 ms).
     */
    void blink(LED_PINS led, uint8_t times = 1, uint16_t delayMs = 100);

private:
    /// Duration for long flash patterns (in milliseconds).
    static constexpr uint16_t LONG_FLASH  = 1000;
    /// Duration for medium flash patterns (in milliseconds).
    static constexpr uint16_t MED_FLASH   = 500;
    /// Duration for short flash patterns (in milliseconds).
    static constexpr uint16_t SHORT_FLASH = 100;

    LEDStrip& _strip;  ///< Reference to the LED strip object

    /**
     * @brief Validates that the provided LED index is within bounds.
     * 
     * @param led The LED to validate.
     * @return true if the LED is valid, false otherwise.
     */
    bool isValidLED(LED_PINS led) const;
};

#endif // INDICATORS_H
