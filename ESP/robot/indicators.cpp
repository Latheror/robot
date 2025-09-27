#include "Indicators.h"

Indicators::Indicators() {
    // Empty constructor
}

void Indicators::init() {
    pinMode(LED1_PIN, OUTPUT);
    pinMode(LED2_PIN, OUTPUT);
    pinMode(LED3_PIN, OUTPUT);
    allOff();
}

void Indicators::setLED(int led, bool state) {
    int pin = LED1_PIN;
    if (led == 1) pin = LED1_PIN;
    else if (led == 2) pin = LED2_PIN;
    else if (led == 3) pin = LED3_PIN;
    else return;

    digitalWrite(pin, state ? HIGH : LOW);
}

void Indicators::allOff() {
    digitalWrite(LED1_PIN, LOW);
    digitalWrite(LED2_PIN, LOW);
    digitalWrite(LED3_PIN, LOW);
}

void Indicators::allOn() {
    digitalWrite(LED1_PIN, HIGH);
    digitalWrite(LED2_PIN, HIGH);
    digitalWrite(LED3_PIN, HIGH);
}

void Indicators::blinkLED(int led, int times, int delayMs) {
    for (int i = 0; i < times; i++) {
        setLED(led, true);
        delay(delayMs);
        setLED(led, false);
        delay(delayMs);
    }
}
