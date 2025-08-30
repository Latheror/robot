/* Sweep
 by BARRAGAN <http://barraganstudio.com>
 This example code is in the public domain.

 modified 8 Nov 2013
 by Scott Fitzgerald
 https://www.arduino.cc/en/Tutorial/LibraryExamples/Sweep
*/

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
// twelve Servo objects can be created on most boards

int pos = 0;    // variable to store the servo position
// Plage de pulse pour servo standard
#define SERVOMIN  150 // valeur minimale du pulse
#define SERVOMAX  600 // valeur maximale du pulse

void setup() {
  delay(2000);
  Serial.begin(115200);
  while (!Serial) { delay(10); } // Attendre que le port série soit prêt
  Serial.println("Init PCA9685 pour servos sur ESP8266...");
  Wire.begin();
  pwm.begin();
  pwm.setPWMFreq(50);  // Fréquence pour servos (50 Hz)

  /* Pour ESP8266, LED_BUILTIN est généralement sur GPIO 2 */
  pinMode(LED_BUILTIN, OUTPUT);
  for (int i = 0; i < 8; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(125);
    digitalWrite(LED_BUILTIN, LOW);
    delay(125);
  }
  digitalWrite(LED_BUILTIN, LOW);

}

void loop() {
  // Pour chaque servo (0 à 3)
  for (int ch = 0; ch < 4; ch++) {
    Serial.print("Contrôle du servo ");
    Serial.println(ch);
    
    // Mouvement de 10 à 170 degrés
    for (pos = 70; pos <= 140; pos += 1) {
      int pulse = map(pos, 0, 180, SERVOMIN, SERVOMAX);
      pwm.setPWM(ch, 0, pulse);
      Serial.print("Canal: ");
      Serial.print(ch);
      Serial.print(" | Position: ");
      Serial.print(pos);
      Serial.print(" | pulse: ");
      Serial.println(pulse);
      delay(15);
    }
    
    // Mouvement de 170 à 10 degrés
    for (pos = 140; pos >= 70; pos -= 1) {
      int pulse = map(pos, 0, 180, SERVOMIN, SERVOMAX);
      pwm.setPWM(ch, 0, pulse);
      Serial.print("Canal: ");
      Serial.print(ch);
      Serial.print(" | Position: ");
      Serial.print(pos);
      Serial.print(" | pulse: ");
      Serial.println(pulse);
      delay(15);
    }
    
    // Pause entre chaque servo
    delay(500);
  }
}

// Function to blink the LED
void blinkLED() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(125);
  digitalWrite(LED_BUILTIN, LOW);
  delay(125);
}
