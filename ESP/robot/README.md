# Robot Control

This folder contains code for controlling servos using an ESP8266 and a PCA9685 board.

## Initial Setup

1. Copy `settings.h.template` to `settings.h`
2. Edit `settings.h` with your WiFi credentials and MQTT broker IP

## Required Libraries

Before uploading the code, install the following libraries through the Arduino Library Manager:

- **Adafruit PWM Servo Driver** (`Adafruit_PWMServoDriver`) - For servo control
- **PubSubClient** - For MQTT communication

The `Wire` library is typically included automatically with ESP8266 support.

Select "Feather ESP8266" or "Generic ESP8266" board in the Arduino IDE.
