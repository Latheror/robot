# Robot Control

This folder contains code for controlling servos using an ESP32 and a PCA9685 board.

## Initial Setup

1. Copy `settings.h.template` to `settings.h`
2. Edit `settings.h` with your WiFi credentials and MQTT broker IP

## Required Libraries

Before uploading the code, install the following libraries through the Arduino Library Manager:

- **Adafruit PWM Servo Driver** (`Adafruit_PWMServoDriver`) - For servo control
- **PubSubClient** - For MQTT communication

The `Wire` library is typically included automatically with ESP32 support.

Select "ESP32 Dev Module" or appropriate ESP32 board in the Arduino IDE.

## Troubleshooting

- **VS Code Include Errors**: If you see "cannot open source file" errors in VS Code, install the Arduino extension and configure the board/intellisense properly.
- **Compilation Issues**: Ensure all required libraries are installed and the correct board is selected.
- **WiFi/MQTT Connection**: Verify credentials in `settings.h` and network accessibility.
