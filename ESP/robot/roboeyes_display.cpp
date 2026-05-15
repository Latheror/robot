/**
 * @file roboeyes_display.cpp
 * @brief Implements OLED eye animations and face-expression control.
 */

#include "roboeyes_display.h"
#include <FluxGarage_RoboEyes.h>

RoboEyesDisplay::RoboEyesDisplay(OLEDDisplay& oledRef)
    : oled(oledRef), lastFrame(0), lastOledUpdate(0), lastChange(0), oledAvailable(false), mqttControlled(false)
{
    // Construct RoboEyes dynamically
    roboEyesPtr = new RoboEyes<Adafruit_SH1106G>(oled.get());
}

void RoboEyesDisplay::begin() {
    auto roboEyes = static_cast<RoboEyes<Adafruit_SH1106G>*>(roboEyesPtr);
    if (!roboEyes) {
        Serial.println("RoboEyes: allocation failed, eye animations disabled");
        oledAvailable = false;
        return;
    }

    oledAvailable = oled.isInitialized();
    if (!oledAvailable) {
        Serial.println("RoboEyes: OLED display not available, eye animations disabled");
        return;
    }

    roboEyes->begin(OledDisplayConfig::SCREEN_WIDTH, OledDisplayConfig::SCREEN_HEIGHT, 100);
    roboEyes->setAutoblinker(true, 3, 2);
    roboEyes->setIdleMode(true, 2, 2);

    // Set default eyes at startup
    setMood(Mood::MOOD_DEFAULT);
    setPosition(Position::POS_DEFAULT);

    lastFrame = millis();
    lastOledUpdate = millis();
    lastChange = millis();

    randomSeed(esp_random());
}

void RoboEyesDisplay::update() {
    if (!oledAvailable) {
        return;
    }

    auto roboEyes = static_cast<RoboEyes<Adafruit_SH1106G>*>(roboEyesPtr);

    unsigned long now = millis();

    if (now - lastFrame >= frameInterval) {
        roboEyes->update();
        lastFrame = now;
    }

    if (now - lastOledUpdate >= oledInterval) {
        oled.get().display();
        lastOledUpdate = now;
    }

    if (now - lastChange >= changeInterval && !mqttControlled) {
        lastChange = now;

        int moodIndex = random(4);
        Mood mood;
        switch (moodIndex) {
            case 0: mood = Mood::MOOD_HAPPY; break;
            case 1: mood = Mood::MOOD_TIRED; break;
            case 2: mood = Mood::MOOD_ANGRY; break;
            default: mood = Mood::MOOD_DEFAULT; break;
        }
        setMood(mood);

        int animIndex = random(3);
        Animation animation;
        switch (animIndex) {
            case 0: animation = Animation::ANIM_BLINK; break;
            case 1: animation = Animation::ANIM_LAUGH; break;
            case 2: animation = Animation::ANIM_CONFUSED; break;
        }
        triggerAnimation(animation);

        Serial.println("RoboEyes: new random mood/animation");
    }
}

bool RoboEyesDisplay::setMood(Mood mood) {
    if (!oledAvailable) {
        return false;
    }

    auto roboEyes = static_cast<RoboEyes<Adafruit_SH1106G>*>(roboEyesPtr);

    switch (mood) {
        case Mood::MOOD_HAPPY:
            roboEyes->setMood(HAPPY);
            Serial.println("RoboEyes: Mood set to HAPPY");
            break;
        case Mood::MOOD_TIRED:
            roboEyes->setMood(TIRED);
            Serial.println("RoboEyes: Mood set to TIRED");
            break;
        case Mood::MOOD_ANGRY:
            roboEyes->setMood(ANGRY);
            Serial.println("RoboEyes: Mood set to ANGRY");
            break;
        case Mood::MOOD_DEFAULT:
            roboEyes->setMood(DEFAULT);
            Serial.println("RoboEyes: Mood set to DEFAULT");
            break;
        default:
            Serial.println("RoboEyes: Unknown mood");
            return false;
    }

    return true;
}

bool RoboEyesDisplay::triggerAnimation(Animation animation) {
    if (!oledAvailable) {
        return false;
    }

    auto roboEyes = static_cast<RoboEyes<Adafruit_SH1106G>*>(roboEyesPtr);

    switch (animation) {
        case Animation::ANIM_BLINK:
            roboEyes->blink();
            Serial.println("RoboEyes: Animation triggered BLINK");
            break;
        case Animation::ANIM_LAUGH:
            roboEyes->anim_laugh();
            Serial.println("RoboEyes: Animation triggered LAUGH");
            break;
        case Animation::ANIM_CONFUSED:
            roboEyes->anim_confused();
            Serial.println("RoboEyes: Animation triggered CONFUSED");
            break;
        default:
            Serial.println("RoboEyes: Unknown animation");
            return false;
    }

    return true;
}

void RoboEyesDisplay::enableMqttControl() {
    mqttControlled = true;
    Serial.println("RoboEyes: MQTT control enabled - stopping automatic random changes");
}

bool RoboEyesDisplay::setPosition(Position position) {
    if (!oledAvailable) {
        return false;
    }

    auto roboEyes = static_cast<RoboEyes<Adafruit_SH1106G>*>(roboEyesPtr);

    switch (position) {
        case Position::POS_N:
            roboEyes->setPosition(N);
            Serial.println("RoboEyes: Position set to N");
            break;
        case Position::POS_NE:
            roboEyes->setPosition(NE);
            Serial.println("RoboEyes: Position set to NE");
            break;
        case Position::POS_E:
            roboEyes->setPosition(E);
            Serial.println("RoboEyes: Position set to E");
            break;
        case Position::POS_SE:
            roboEyes->setPosition(SE);
            Serial.println("RoboEyes: Position set to SE");
            break;
        case Position::POS_S:
            roboEyes->setPosition(S);
            Serial.println("RoboEyes: Position set to S");
            break;
        case Position::POS_SW:
            roboEyes->setPosition(SW);
            Serial.println("RoboEyes: Position set to SW");
            break;
        case Position::POS_W:
            roboEyes->setPosition(W);
            Serial.println("RoboEyes: Position set to W");
            break;
        case Position::POS_NW:
            roboEyes->setPosition(NW);
            Serial.println("RoboEyes: Position set to NW");
            break;
        case Position::POS_DEFAULT:
            roboEyes->setPosition(DEFAULT);
            Serial.println("RoboEyes: Position set to DEFAULT");
            break;
        default:
            Serial.println("RoboEyes: Unknown position");
            return false;
    }

    return true;
}

bool RoboEyesDisplay::setCuriosity(bool enabled) {
    if (!oledAvailable) {
        return false;
    }

    auto roboEyes = static_cast<RoboEyes<Adafruit_SH1106G>*>(roboEyesPtr);
    roboEyes->setCuriosity(enabled);
    Serial.printf("RoboEyes: Curiosity set to %s\n", enabled ? "ON" : "OFF");
    return true;
}

bool RoboEyesDisplay::setSweat(bool enabled) {
    if (!oledAvailable) {
        return false;
    }

    auto roboEyes = static_cast<RoboEyes<Adafruit_SH1106G>*>(roboEyesPtr);
    roboEyes->setSweat(enabled);
    Serial.printf("RoboEyes: Sweat set to %s\n", enabled ? "ON" : "OFF");
    return true;
}

bool RoboEyesDisplay::setHFlicker(bool enabled, uint8_t amplitude) {
    if (!oledAvailable) {
        return false;
    }

    auto roboEyes = static_cast<RoboEyes<Adafruit_SH1106G>*>(roboEyesPtr);
    roboEyes->setHFlicker(enabled, amplitude);
    Serial.printf("RoboEyes: H-Flicker set to %s (amplitude: %d)\n", enabled ? "ON" : "OFF", amplitude);
    return true;
}

bool RoboEyesDisplay::setVFlicker(bool enabled, uint8_t amplitude) {
    if (!oledAvailable) {
        return false;
    }

    auto roboEyes = static_cast<RoboEyes<Adafruit_SH1106G>*>(roboEyesPtr);
    roboEyes->setVFlicker(enabled, amplitude);
    Serial.printf("RoboEyes: V-Flicker set to %s (amplitude: %d)\n", enabled ? "ON" : "OFF", amplitude);
    return true;
}

bool RoboEyesDisplay::setAutoblinker(bool enabled, int interval, int variation) {
    if (!oledAvailable) {
        return false;
    }

    auto roboEyes = static_cast<RoboEyes<Adafruit_SH1106G>*>(roboEyesPtr);
    roboEyes->setAutoblinker(enabled, interval, variation);
    Serial.printf("RoboEyes: Autoblinker set to %s (interval: %d, variation: %d)\n", enabled ? "ON" : "OFF", interval, variation);
    return true;
}

bool RoboEyesDisplay::setIdleMode(bool enabled, int interval, int variation) {
    if (!oledAvailable) {
        return false;
    }

    auto roboEyes = static_cast<RoboEyes<Adafruit_SH1106G>*>(roboEyesPtr);
    roboEyes->setIdleMode(enabled, interval, variation);
    Serial.printf("RoboEyes: Idle mode set to %s (interval: %d, variation: %d)\n", enabled ? "ON" : "OFF", interval, variation);
    return true;
}

bool RoboEyesDisplay::openEyes(bool left, bool right) {
    if (!oledAvailable) {
        return false;
    }

    auto roboEyes = static_cast<RoboEyes<Adafruit_SH1106G>*>(roboEyesPtr);
    if (left && right) {
        roboEyes->open();
        Serial.println("RoboEyes: Both eyes opened");
    } else if (left) {
        roboEyes->open(1, 0);
        Serial.println("RoboEyes: Left eye opened");
    } else if (right) {
        roboEyes->open(0, 1);
        Serial.println("RoboEyes: Right eye opened");
    }
    return true;
}

bool RoboEyesDisplay::closeEyes(bool left, bool right) {
    if (!oledAvailable) {
        return false;
    }

    auto roboEyes = static_cast<RoboEyes<Adafruit_SH1106G>*>(roboEyesPtr);
    if (left && right) {
        roboEyes->close();
        Serial.println("RoboEyes: Both eyes closed");
    } else if (left) {
        roboEyes->close(1, 0);
        Serial.println("RoboEyes: Left eye closed");
    } else if (right) {
        roboEyes->close(0, 1);
        Serial.println("RoboEyes: Right eye closed");
    }
    return true;
}
