#include "oled_display.h"

// Définition de l’instance globale
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void initOLED() {
  Wire.begin(SDA_PIN, SCL_PIN, 100000); // I2C standard 100 kHz
  delay(100);

  if (!display.begin(OLED_ADDR, true)) {
    Serial.println(F("Échec initialisation SH1106"));
    for (;;); // blocage si écran non trouvé
  }

  display.clearDisplay();
  display.display();
}
