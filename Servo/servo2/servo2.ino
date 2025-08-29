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
  Wire.begin();
  pwm.begin();
  pwm.setPWMFreq(50);  // Fréquence pour servos (50 Hz)

  /* Pour ESP8266, LED_BUILTIN est généralement sur GPIO 2 */
  // pinMode(LED_BUILTIN, OUTPUT);
  // for (int i = 0; i < 8; i++) {
  //   digitalWrite(LED_BUILTIN, HIGH);
  //   delay(125);
  //   digitalWrite(LED_BUILTIN, LOW);
  //   delay(125);
  // }
  // digitalWrite(LED_BUILTIN, LOW);

}

void loop() {
  for (pos = 0; pos <= 180; pos += 1) {
    int pulse = map(pos, 0, 180, SERVOMIN, SERVOMAX);
    pwm.setPWM(0, 0, pulse); // Servo 1 sur canal 0
    pwm.setPWM(1, 0, pulse); // Servo 2 sur canal 1
    delay(15);
  }
  for (pos = 180; pos >= 0; pos -= 1) {
    int pulse = map(pos, 0, 180, SERVOMIN, SERVOMAX);
    pwm.setPWM(0, 0, pulse);
    pwm.setPWM(1, 0, pulse);
    delay(15);
  }
}

// Function to blink the LED
void blinkLED() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(125);
  digitalWrite(LED_BUILTIN, LOW);
  delay(125);
}
