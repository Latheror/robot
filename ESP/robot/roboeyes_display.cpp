#include "roboeyes_display.h"
#include "oled_display.h"

#include <Wire.h>
#include <Adafruit_SSD1306.h>

// ⚠️ Fix conflict with DEFAULT
#ifdef DEFAULT
#undef DEFAULT
#endif
#include "FluxGarage_RoboEyes.h"

// External display instance from oled_display.cpp
extern Adafruit_SSD1306 display;

// Correct RoboEyes instantiation with display
RoboEyes<Adafruit_SSD1306> roboEyes(display);

// Frame timing
unsigned long lastFrame = 0;
const unsigned long frameInterval = 10; // ~100 fps max

// Animation timing
unsigned long lastChange = 0;
const unsigned long changeInterval = 2000; // 2 seconds

void initRoboEyes() {
  roboEyes.begin(SCREEN_WIDTH, SCREEN_HEIGHT, 100);

  // Example base settings
  roboEyes.setAutoblinker(true, 3, 2);
  roboEyes.setIdleMode(true, 2, 2);

  lastFrame = millis();
  lastChange = millis();

  // Seed randomness
  randomSeed(analogRead(A0));
}

void handleRoboEyes() {
  unsigned long now = millis();

  // --- Update eyes at steady framerate ---
  if (now - lastFrame >= frameInterval) {
    roboEyes.update();
    lastFrame = now;
  }

  // --- Change mood/animation randomly every 2 seconds ---
  if (now - lastChange >= changeInterval) {
    lastChange = now;

    // Pick a random mood
    int mood = random(4); // 0..3
    switch (mood) {
      case 0: roboEyes.setMood(HAPPY); break;
      case 1: roboEyes.setMood(TIRED); break;
      case 2: roboEyes.setMood(ANGRY); break;
      default: roboEyes.setMood(DEFAULT); break;
    }

    // Pick a random animation
    int anim = random(3); // 0..2
    switch (anim) {
      case 0: roboEyes.blink(); break;
      case 1: roboEyes.anim_laugh(); break;
      case 2: roboEyes.anim_confused(); break;
    }

    Serial.println("RoboEyes: new random mood/animation");
  }
}
