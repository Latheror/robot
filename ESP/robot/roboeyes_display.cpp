#include "roboeyes_display.h"
#include <FluxGarage_RoboEyes.h>

RoboEyesDisplay::RoboEyesDisplay(OLEDDisplay& oledRef)
    : oled(oledRef), lastFrame(0), lastOledUpdate(0), lastChange(0)
{
    // Construct RoboEyes dynamically
    roboEyesPtr = new RoboEyes<Adafruit_SH1106G>(oled.get());
}

void RoboEyesDisplay::begin() {
    auto roboEyes = static_cast<RoboEyes<Adafruit_SH1106G>*>(roboEyesPtr);

    roboEyes->begin(SCREEN_WIDTH, SCREEN_HEIGHT, 100);
    roboEyes->setAutoblinker(true, 3, 2);
    roboEyes->setIdleMode(true, 2, 2);

    lastFrame = millis();
    lastOledUpdate = millis();
    lastChange = millis();

    randomSeed(esp_random());
}

void RoboEyesDisplay::update() {
    auto roboEyes = static_cast<RoboEyes<Adafruit_SH1106G>*>(roboEyesPtr);

    unsigned long now = millis();

    if (now - lastFrame >= frameInterval) {
        roboEyes->update();
        lastFrame = now;
    }

    if (now - lastOledUpdate >= oledInterval) {
        if (isI2CAvailable(OLED_ADDR)) {
            oled.get().display();
        } else {
            Serial.println("OLED not detected on I2C!");
        }
        lastOledUpdate = now;
    }

    if (now - lastChange >= changeInterval) {
        lastChange = now;

        int mood = random(4);
        switch (mood) {
            case 0: roboEyes->setMood(HAPPY); break;
            case 1: roboEyes->setMood(TIRED); break;
            case 2: roboEyes->setMood(ANGRY); break;
            default: roboEyes->setMood(DEFAULT); break;
        }

        int anim = random(3);
        switch (anim) {
            case 0: roboEyes->blink(); break;
            case 1: roboEyes->anim_laugh(); break;
            case 2: roboEyes->anim_confused(); break;
        }

        Serial.println("RoboEyes: new random mood/animation");
    }
}

bool RoboEyesDisplay::isI2CAvailable(uint8_t address) {
    Wire.beginTransmission(address);
    return (Wire.endTransmission() == 0);
}
