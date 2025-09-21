#include "roboeyes_display.h"
#include "oled_display.h"

#include <Wire.h>
#include <Adafruit_SH110X.h>  // Use SH1106 driver

// Fix conflict with DEFAULT
#ifdef DEFAULT
#undef DEFAULT
#endif
#include "FluxGarage_RoboEyes.h"

// I2C OLED address
#define OLED_ADDR 0x3C

// Display instance declared in oled_display.cpp
extern Adafruit_SH1106G display;

// Create RoboEyes instance with the display
RoboEyes<Adafruit_SH1106G> roboEyes(display);

// Timers
unsigned long lastFrame = 0;
const unsigned long frameInterval = 10;   // logic update ~100 FPS

unsigned long lastOledUpdate = 0;
const unsigned long oledInterval = 100;   // OLED refresh ~10 FPS

unsigned long lastChange = 0;
const unsigned long changeInterval = 10000; // change mood/animation every 10s

// Check if I2C device is available
bool isI2CAvailable(uint8_t address) {
  Wire.beginTransmission(address);
  return (Wire.endTransmission() == 0); // 0 = ACK
}

void initRoboEyes() {
  roboEyes.begin(SCREEN_WIDTH, SCREEN_HEIGHT, 100);

  // Basic settings
  roboEyes.setAutoblinker(true, 3, 2);
  roboEyes.setIdleMode(true, 2, 2);

  lastFrame = millis();
  lastOledUpdate = millis();
  lastChange = millis();

  // Random seed
  randomSeed(analogRead(A0));
}

void handleRoboEyes() {
  unsigned long now = millis();

  // --- Fast eye logic update ---
  if (now - lastFrame >= frameInterval) {
    roboEyes.update();
    lastFrame = now;
  }

  // --- Limited OLED refresh ---
  if (now - lastOledUpdate >= oledInterval) {
    if (isI2CAvailable(OLED_ADDR)) {
      display.display();
    } else {
      Serial.println("⚠️ OLED not detected on I2C!");
    }
    lastOledUpdate = now;
  }

  // --- Change mood/animation every 2s ---
  if (now - lastChange >= changeInterval) {
    lastChange = now;

    // Random mood
    int mood = random(4);
    switch (mood) {
      case 0: roboEyes.setMood(HAPPY); break;
      case 1: roboEyes.setMood(TIRED); break;
      case 2: roboEyes.setMood(ANGRY); break;
      default: roboEyes.setMood(DEFAULT); break;
    }

    // Random animation
    int anim = random(3);
    switch (anim) {
      case 0: roboEyes.blink(); break;
      case 1: roboEyes.anim_laugh(); break;
      case 2: roboEyes.anim_confused(); break;
    }

    Serial.println("RoboEyes: new random mood/animation");
  }
}
