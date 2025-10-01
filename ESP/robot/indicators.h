#ifndef INDICATORS_H
#define INDICATORS_H

#include <Arduino.h>
#include <array>

class Indicators {
public:
    // LED identifiers
    enum class LED {
        STATUS = 0,   // General status LED
        NETWORK,      // Network connection status
        ACTIVITY,     // General activity indicator
        COUNT
    };

    // Initialize the indicator system
    bool begin();

    // Control methods
    void set(LED led, bool state);
    void setAll(bool state);
    void toggle(LED led);
    void blink(LED led, uint8_t times = 1, uint16_t delayMs = 100);
    
    // Pattern methods
    void flashSuccess();  // Three quick blinks
    void flashError();    // One long flash
    void flashWarning();  // Two medium flashes

private:
    static constexpr uint8_t PIN_MAP[] = {4, 5, 6};  // LED pin assignments
    static constexpr uint16_t LONG_FLASH = 1000;     // Long flash duration (ms)
    static constexpr uint16_t MED_FLASH = 500;       // Medium flash duration (ms)
    static constexpr uint16_t SHORT_FLASH = 100;     // Short flash duration (ms)
    
    bool isValidLED(LED led) const;
    uint8_t getLEDPin(LED led) const;
};

#endif // INDICATORS_H
