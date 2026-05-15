/**
 * @file rgb_led.h
 * @brief Single onboard NeoPixel RGB LED wrapper.
 */

#ifndef RGB_LED_H
#define RGB_LED_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

/**
 * @class RGBLed
 * @brief Manages a single RGB LED using Adafruit NeoPixel.
 */
class RGBLed {
public:
    static constexpr uint8_t RGB_LED_PIN = 48;      ///< RGB LED pin number
    static constexpr uint8_t SINGLE_LED_COUNT = 1;  ///< Number of LEDs (always 1)

    /**
     * @brief Constructor.
     */
    RGBLed() : _pixel(SINGLE_LED_COUNT, RGB_LED_PIN, NEO_GRB + NEO_KHZ800) {}

    /**
     * @brief Initializes the RGB LED.
     * @return true if successful.
     */
    bool begin();

    /**
     * @brief Sets the color of the LED.
     * @param r Red component (0-255).
     * @param g Green component (0-255).
     * @param b Blue component (0-255).
     */
    void setColor(uint8_t r, uint8_t g, uint8_t b);

    /**
     * @brief Clears the LED (turns off).
     */
    void clear();

    /**
     * @brief Sets the brightness of the LED.
     * @param brightness Brightness level (0-255).
     */
    void setBrightness(uint8_t brightness);

private:
    Adafruit_NeoPixel _pixel; ///< NeoPixel object
};

// Inline method definitions
inline bool RGBLed::begin() {
    _pixel.begin();
    _pixel.clear();
    _pixel.show();
    return true;
}

inline void RGBLed::setColor(uint8_t r, uint8_t g, uint8_t b) {
    _pixel.setPixelColor(0, _pixel.Color(r, g, b));
    _pixel.show();
}

inline void RGBLed::clear() {
    _pixel.clear();
    _pixel.show();
}

inline void RGBLed::setBrightness(uint8_t brightness) {
    _pixel.setBrightness(brightness);
    _pixel.show();
}

#endif // RGB_LED_H
