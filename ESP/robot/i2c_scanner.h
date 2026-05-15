/**
 * @file i2c_scanner.h
 * @brief I2C bus discovery utility for hardware diagnostics.
 */

#ifndef I2C_SCANNER_H
#define I2C_SCANNER_H

#include <Arduino.h>

/**
 * @brief Scans the I2C bus and prints all detected devices.
 * 
 * This function probes all valid I2C addresses (1-126) and reports
 * any devices that respond. Useful for debugging hardware connectivity.
 */
void i2cScan();

#endif // I2C_SCANNER_H
