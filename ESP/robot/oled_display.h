#pragma once
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SDA_PIN 21
#define SCL_PIN 22
#define OLED_ADDR 0x3C

// Déclaration globale accessible ailleurs
extern Adafruit_SSD1306 display;

// Fonctions publiques
void initOLED();
