#include "roboeyes_display.h"
#include "oled_display.h"

#include <Wire.h>
#include <Adafruit_SH110X.h>  // ✅ Remplace SSD1306

// ⚠️ Fix conflict with DEFAULT
#ifdef DEFAULT
#undef DEFAULT
#endif
#include "FluxGarage_RoboEyes.h"

// Adresse I2C OLED
#define OLED_ADDR 0x3C

// Instance display déclarée dans oled_display.cpp
extern Adafruit_SH1106G display;   // ✅ SH1106 au lieu de SSD1306

// Création RoboEyes avec l’écran
RoboEyes<Adafruit_SH1106G> roboEyes(display);  // ✅

 // Timers
unsigned long lastFrame = 0;
const unsigned long frameInterval = 10;   // logique des yeux = ~100 FPS max

unsigned long lastOledUpdate = 0;
const unsigned long oledInterval = 100;   // rafraîchissement écran = ~10 FPS

unsigned long lastChange = 0;
const unsigned long changeInterval = 2000; // changer humeur/anim toutes les 2s

// Vérifie si périphérique I2C répond
bool isI2CAvailable(uint8_t address) {
  Wire.beginTransmission(address);
  return (Wire.endTransmission() == 0); // 0 = ACK
}

void initRoboEyes() {
  roboEyes.begin(SCREEN_WIDTH, SCREEN_HEIGHT, 100);

  // Paramètres de base
  roboEyes.setAutoblinker(true, 3, 2);
  roboEyes.setIdleMode(true, 2, 2);

  lastFrame = millis();
  lastOledUpdate = millis();
  lastChange = millis();

  // Seed aléatoire
  randomSeed(analogRead(A0));
}

void handleRoboEyes() {
  unsigned long now = millis();

  // --- Update logique des yeux (rapide) ---
  if (now - lastFrame >= frameInterval) {
    roboEyes.update();
    lastFrame = now;
  }

  // --- Rafraîchissement limité de l’OLED ---
  if (now - lastOledUpdate >= oledInterval) {
    if (isI2CAvailable(OLED_ADDR)) {
      display.display();
    } else {
      Serial.println("⚠️ OLED non détecté sur I2C !");
    }
    lastOledUpdate = now;
  }

  // --- Changement humeur/animation toutes les 2s ---
  if (now - lastChange >= changeInterval) {
    lastChange = now;

    // Choisir humeur aléatoire
    int mood = random(4);
    switch (mood) {
      case 0: roboEyes.setMood(HAPPY); break;
      case 1: roboEyes.setMood(TIRED); break;
      case 2: roboEyes.setMood(ANGRY); break;
      default: roboEyes.setMood(DEFAULT); break;
    }

    // Choisir animation aléatoire
    int anim = random(3);
    switch (anim) {
      case 0: roboEyes.blink(); break;
      case 1: roboEyes.anim_laugh(); break;
      case 2: roboEyes.anim_confused(); break;
    }

    Serial.println("RoboEyes: new random mood/animation");
  }
}
