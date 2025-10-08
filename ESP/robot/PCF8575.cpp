#include "PCF8575.h"

// Uncomment to enable debug messages
#define DEBUG_PCF8575

/**
 * @brief Construct a new PCF8575 object
 * 
 * @param address I2C address of the PCF8575 (0x20 to 0x27)
 * @param wirePort Reference to the TwoWire instance (Wire)
 */
PCF8575::PCF8575(uint8_t address, TwoWire &wirePort)
    : _address(address), _wire(&wirePort), _outputState(0xFFFF) // Default high (inputs)
{
}

/**
 * @brief Initialize I2C and set all pins high (inputs)
 * 
 * @return true if initialization succeeded
 * @return false if I2C write failed
 */
bool PCF8575::begin() {
#ifdef DEBUG_PCF8575
    Serial.println("[PCF8575] begin");
#endif
    return write16(_outputState); // Initialize outputs high
}

/**
 * @brief Write a 16-bit value to all PCF8575 pins
 * 
 * @param value 16-bit output value
 * @return true if I2C transmission succeeded
 * @return false if I2C transmission failed
 */
bool PCF8575::write16(uint16_t value) {
    _outputState = value;
    _wire->beginTransmission(_address);
    _wire->write(_outputState & 0xFF);        // Low byte
    _wire->write((_outputState >> 8) & 0xFF); // High byte
    uint8_t error = _wire->endTransmission();
#ifdef DEBUG_PCF8575
if (error != 0)
{
    Serial.print("[PCF8575] write16 error=");
    Serial.print(error);
    Serial.print(" while writing 0x");
    Serial.println(_outputState, HEX);
}
#endif

    return (error == 0);
}

/**
 * @brief Read a 16-bit value from all PCF8575 pins
 * 
 * @return uint16_t Current pin states (1 = high, 0 = low)
 */
uint16_t PCF8575::read16() {
    _wire->requestFrom(_address, (uint8_t)2);
    uint16_t value = 0xFFFF;
    if (_wire->available() >= 2) {
        uint8_t low = _wire->read();
        uint8_t high = _wire->read();
        value = (high << 8) | low;
    }
    return value;
}

/**
 * @brief Write a single pin high or low
 * 
 * @param pin Pin number (0-15)
 * @param value true = HIGH, false = LOW
 * @return true if I2C transmission succeeded
 * @return false if pin number invalid or I2C failed
 */
bool PCF8575::writePin(uint8_t pin, bool value) {
    if (pin > 15) return false;

    if (value)
        _outputState |= (1 << pin);
    else
        _outputState &= ~(1 << pin);

#ifdef DEBUG_PCF8575
    Serial.print("[PCF8575] Set pin ");
    Serial.print(pin);
    Serial.print(" to ");
    Serial.println(value ? "HIGH" : "LOW");
#endif

    return write16(_outputState);
}

/**
 * @brief Read a single pin state
 * 
 * @param pin Pin number (0-15)
 * @return true if HIGH
 * @return false if LOW or pin number invalid
 */
bool PCF8575::readPin(uint8_t pin) {
    if (pin > 15) return false;
    uint16_t state = read16();
    return (state & (1 << pin)) != 0;
}
