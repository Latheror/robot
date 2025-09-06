#include "roboeyes_display.h"
#include "oled_display.h"

#include <Wire.h>
#include <Adafruit_SSD1306.h>

// ⚠️ Corrige le conflit DEFAULT
#ifdef DEFAULT
#undef DEFAULT
#endif
#include "FluxGarage_RoboEyes.h"

// Déclaration de l’objet écran venant de oled_display.cpp
extern Adafruit_SSD1306 display;

// ⚠️ Instanciation CORRECTE de RoboEyes avec l’écran
RoboEyes<Adafruit_SSD1306> roboEyes(display);

// Timer pour alterner les animations
unsigned long lastChange = 0;
const unsigned long changeInterval = 2000; // 2 secondes
bool toggle = false;

void initRoboEyes() {
  // Démarrage de RoboEyes avec la taille de l’écran et un framerate max
  roboEyes.begin(SCREEN_WIDTH, SCREEN_HEIGHT, 100);

  // Réglages d’exemple
  roboEyes.setAutoblinker(true, 3, 2);
  roboEyes.setIdleMode(true, 2, 2);
  roboEyes.setMood(DEFAULT);

  lastChange = millis();
}

void handleRoboEyes() {

  Serial.println("Updating RoboEyes...");

  roboEyes.update();

  unsigned long now = millis();
  if (now - lastChange >= changeInterval) {
    lastChange = now;
    toggle = !toggle;

    if (toggle) {
      roboEyes.setMood(HAPPY);
      roboEyes.anim_laugh();
    } else {
      roboEyes.setMood(TIRED);
      roboEyes.blink();
    }
  }
}
