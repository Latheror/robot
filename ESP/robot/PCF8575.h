#ifndef PCF8575_H
#define PCF8575_H

#include <Arduino.h>
#include <Wire.h>

class PCF8575 {
public:
    PCF8575(uint8_t address, TwoWire &wirePort = Wire);

    void begin();
    void write16(uint16_t value);
    uint16_t read16();
    void writePin(uint8_t pin, bool value);
    bool readPin(uint8_t pin);

private:
    uint8_t _address;
    TwoWire* _wire;
    uint16_t _outputState;
};

#endif
