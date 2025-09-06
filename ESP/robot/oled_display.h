#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <Arduino.h>

// Dimensions de l'écran (adapter selon ton module : 128x32 ou 128x64)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Initialise l’écran OLED
void initOLED();

// Affiche un texte simple
void displayText(const char* text);

#endif
