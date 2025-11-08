#pragma once
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define LED_STRIP_PIN 13
#define STRIP_LED_COUNT 5
#define LED_BRIGHTNESS 3

/**
 * @enum LedName
 * @brief Defines LED identifiers for the strip.
 */
enum LedName {
    LED_NAME_1 = 0,
    LED_NAME_2,
    LED_NAME_3,
    LED_NAME_4,
    LED_NAME_5
};

/**
 * @class LEDStrip
 * @brief Manages an Adafruit NeoPixel LED strip for status indicators.
 */
class LEDStrip {
public:
    /**
     * @brief Constructor for LEDStrip.
     * @param pin The GPIO pin connected to the LED strip.
     * @param numLeds The number of LEDs in the strip.
     */
    LEDStrip(uint8_t pin = LED_STRIP_PIN, uint8_t numLeds = STRIP_LED_COUNT);

    /**
     * @brief Initializes the LED strip.
     */
    void begin();

    /**
     * @brief Sets the color of a specific LED.
     * @param led The LED to set.
     * @param color The color value.
     */
    void setColor(LedName led, uint32_t color);

    /**
     * @brief Sets all LEDs to the same color.
     * @param color The color value.
     */
    void setAll(uint32_t color);

    /**
     * @brief Updates the LED strip with current colors.
     */
    void show();

    /**
     * @brief Blinks all LEDs with a color for a delay.
     * @param color The blink color.
     * @param delayMs The blink duration in milliseconds.
     */
    void blink(uint32_t color, uint16_t delayMs);

    /**
     * @brief Displays a rainbow effect.
     */
    void rainbow();

    /**
     * @brief Sets the brightness of the strip.
     * @param brightness The brightness level (0-255).
     */
    void setBrightness(uint8_t brightness);

    /**
     * @brief Creates a color value from RGB components.
     * @param r Red component.
     * @param g Green component.
     * @param b Blue component.
     * @return The color value.
     */
    uint32_t color(uint8_t r, uint8_t g, uint8_t b);

private:
    uint8_t _pin;
    uint8_t _numLeds;
    uint8_t _hue;
    Adafruit_NeoPixel _strip;
};
