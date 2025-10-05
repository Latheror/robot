#ifndef PCF8575_H
#define PCF8575_H

#include <Arduino.h>
#include <Wire.h>

/**
 * @brief PCF8575 16-bit I2C GPIO Expander
 */
class PCF8575 {
public:
    /**
     * @brief Construct a new PCF8575 object
     * 
     * @param address I2C address of the PCF8575 (0x20 to 0x27)
     * @param wirePort Reference to the TwoWire instance (default: Wire)
     */
    PCF8575(uint8_t address, TwoWire &wirePort = Wire);

    /**
     * @brief Initialize I2C and set all pins high (inputs)
     * 
     * @return true if initialization succeeded
     * @return false if I2C write failed
     */
    bool begin();

    /**
     * @brief Write a 16-bit value to all PCF8575 pins
     * 
     * @param value 16-bit output value
     * @return true if I2C transmission succeeded
     * @return false if I2C transmission failed
     */
    bool write16(uint16_t value);

    /**
     * @brief Read a 16-bit value from all PCF8575 pins
     * 
     * @return uint16_t Current pin states (1 = high, 0 = low)
     */
    uint16_t read16();

    /**
     * @brief Write a single pin high or low
     * 
     * @param pin Pin number (0-15)
     * @param value true = HIGH, false = LOW
     * @return true if I2C transmission succeeded
     * @return false if pin number invalid or I2C failed
     */
    bool writePin(uint8_t pin, bool value);

    /**
     * @brief Read a single pin state
     * 
     * @param pin Pin number (0-15)
     * @return true if HIGH
     * @return false if LOW or pin number invalid
     */
    bool readPin(uint8_t pin);

private:
    uint8_t _address;        ///< I2C address of the PCF8575
    TwoWire* _wire;          ///< Pointer to TwoWire instance
    uint16_t _outputState;   ///< Cached output state
};

#endif
