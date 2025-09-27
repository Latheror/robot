#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "oled_display.h"

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- I2C Scan Function ---
void scanI2C() {
  byte error, address;
  int nDevices = 0;

  Serial.println("Scanning I2C bus...");

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println(" !");
      nDevices++;
    }
  }

  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  } else {
    Serial.println("I2C scan complete\n");
  }
}

// --- OLED Initialization ---
void initOLED() {
  Wire.begin(SDA_PIN, SCL_PIN, 100000); // SDA, SCL, frequency
  delay(100);

  scanI2C(); // run scan before initializing the display

  if (!display.begin(OLED_ADDR, true)) {
    Serial.println(F("Failed to initialize SH1106"));
    for (;;); // stop if the display is not found
  }

  display.clearDisplay();
  display.display();
}