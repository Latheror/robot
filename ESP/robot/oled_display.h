#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <Arduino.h>

// Initialise l’écran OLED
void initOLED();

// Affiche un texte simple
void displayText(const char* text);

#endif
