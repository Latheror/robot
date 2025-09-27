#pragma once
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>   // ✅ driver pour SH1106

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SDA_PIN 17
#define SCL_PIN 18
#define OLED_ADDR 0x3C

// Déclaration globale de l'écran
extern Adafruit_SH1106G display;

// Fonctions publiques
void initOLED();
