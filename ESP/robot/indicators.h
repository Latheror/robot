#ifndef INDICATORS_H
#define INDICATORS_H

#include <Arduino.h>

/**
 * @class Indicators
 * @brief Manages the system's LED indicators through a PCF8575 I/O expander.
 *
 * This class provides control over several indicator LEDs such as status, network,
 * and activity. Each LED can be turned on/off, toggled, or blinked with custom patterns.
 */
class Indicators {
public:
    /**
     * @enum LED_PINS
     * @brief Defines LED identifiers mapped directly to hardware pin numbers.
     */
    enum LED_PINS : uint8_t {
        WIFI   = 0,  ///< General system status LED (Pin 0)
        MQTT  = 1,  ///< Network connection indicator (Pin 1)
        MESSAGE_SEND = 2,  ///< General activity indicator (Pin 2)
        MOTORS_MOVING = 3,  ///< Motor activity indicator (Pin 3)
        IS_RECORDING = 4,  ///< Voice recording indicator (Pin 4)
        COUNT          ///< Number of available LEDs
    };

    /**
     * @brief Initializes the indicator system and the PCF8575 device.
     * 
     * @return true if initialization was successful, false otherwise.
     */
    bool begin();

    /**
     * @brief Sets the state of a specific LED.
     * 
     * @param led The LED to modify (from LED_PINS enum).
     * @param state True to turn the LED on, false to turn it off.
     */
    void set(LED_PINS led, bool state);

    /**
     * @brief Sets the state of all LEDs at once.
     * 
     * @param state True to turn all LEDs on, false to turn them off.
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

    /**
     * @brief Displays a "success" feedback pattern (three short blinks).
     */
    void flashSuccess();

    /**
     * @brief Displays an "error" feedback pattern (one long blink).
     */
    void flashError();

    /**
     * @brief Displays a "warning" feedback pattern (two medium blinks).
     */
    void flashWarning();

private:
    /// Duration for long flash patterns (in milliseconds).
    static constexpr uint16_t LONG_FLASH  = 1000;
    /// Duration for medium flash patterns (in milliseconds).
    static constexpr uint16_t MED_FLASH   = 500;
    /// Duration for short flash patterns (in milliseconds).
    static constexpr uint16_t SHORT_FLASH = 100;

    /**
     * @brief Validates that the provided LED index is within bounds.
     * 
     * @param led The LED to validate.
     * @return true if the LED is valid, false otherwise.
     */
    bool isValidLED(LED_PINS led) const;
};

#endif // INDICATORS_H
