/**
 * @file i2c_scanner.cpp
 * @brief Scans the configured I2C bus and logs discovered device addresses.
 */

#include "i2c_scanner.h"
#include <Wire.h>

void i2cScan() {
    Serial.println("\n[I2C] Scanning bus for connected devices...");
    
    byte error, address;
    int nDevices = 0;

    for (address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0) {
            Serial.print("[I2C] Device found at address 0x");
            if (address < 16) Serial.print("0");
            Serial.print(address, HEX);
            Serial.print(" (");
            Serial.print(address);
            Serial.println(")");
            nDevices++;
        }
    }

    if (nDevices == 0) {
        Serial.println("[I2C] No devices found!");
    } else {
        Serial.printf("[I2C] Scan complete - %d device(s) found\n\n", nDevices);
    }
}
