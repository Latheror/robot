#include "oled_display.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Dimensions de l'écran (adapte selon ton module : 128x32 ou 128x64)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Adresse I2C par défaut de l’OLED (0x3C la plupart du temps)
#define OLED_ADDR   0x3C

// Création de l'objet écran
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

void initOLED() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("Erreur : écran OLED non détecté !"));
    for (;;); // Boucle infinie si pas d’écran
  }
  display.clearDisplay();
  display.setTextSize(1);      // Taille du texte
  display.setTextColor(SSD1306_WHITE);  
  display.setCursor(0, 0);
  display.println(F("OLED Init OK"));
  display.display();
  delay(1000);
}

void displayText(const char* text) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(text);
  display.display();
}
