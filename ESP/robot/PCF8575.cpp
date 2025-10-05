#include "PCF8575.h"

PCF8575::PCF8575(uint8_t address, TwoWire &wirePort)
    : _address(address), _wire(&wirePort), _outputState(0xFFFF) // Default high (inputs)
{
}

void PCF8575::begin() {

    Serial.print("[PCF8575} begin");

    _wire->begin();
    write16(_outputState); // Initialize outputs high
}

void PCF8575::write16(uint16_t value) {
    _outputState = value;
    _wire->beginTransmission(_address);
    _wire->write(_outputState & 0xFF);         // Low byte
    _wire->write((_outputState >> 8) & 0xFF);  // High byte
    _wire->endTransmission();
}

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

void PCF8575::writePin(uint8_t pin, bool value) {

    Serial.print("[PCF8575} Set pin ");
    Serial.print(pin);
    Serial.print(" to ");
    Serial.println(value ? "HIGH" : "LOW");

    if (pin > 15) return;
    if (value)
        _outputState |= (1 << pin);
    else
        _outputState &= ~(1 << pin);
    write16(_outputState);
}

bool PCF8575::readPin(uint8_t pin) {
    if (pin > 15) return false;
    uint16_t state = read16();
    return (state & (1 << pin)) != 0;
}
