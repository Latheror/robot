# ESP32 Robot Hardware & Firmware Setup

## Overview

The ESP32 is the brain of the robot, handling sensors, displays, and executing commands. This guide covers hardware setup and firmware installation.

**Location**: `ESP/robot/robot.ino`

## Hardware Components

### Core Microcontroller
- **ESP32-WROOM-32** Development Board
- Dual-core processor at 240 MHz
- 320 KB RAM + 4 MB Flash
- Built-in WiFi & Bluetooth

### Audio

**Microphone**:
- **INMP441** I2S Digital Microphone
  - Pins: CLK, WS, SD (I2S interface)
  - Voltage: 3.3V
  
**Speaker**:
- **MAX98357A** Audio Amplifier + Speaker
  - Input: I2S digital audio
  - Output: 3W speaker
  - Pins: BCLK, LRCLK, DIN (I2S interface)

### Display & Visual Indicators

**OLED Display**:
- 1.3" SSD1306 Display (128x64 pixels)
- Interface: I2C
- Pins: SDA (GPIO 21), SCL (GPIO 22)

**RGB LEDs**:
- WS2812B Addressable LEDs
- Control Pin: GPIO 4
- Voltage: 5V with level shifter

**RoboEyes Display** (optional):
- 16x16 LED matrix animation
- Shows emotions and expressions

### Mechanical Control

**Servo Motors**:
- Control via PWM pins
- Typical pins: GPIO 12, 13, 14, 15
- Voltage: 5V regulated

## Wiring Diagram

```
ESP32 Pinout:

                    +---+---+
                USB | o | o | 3V3
                GND | o | o | EN
                D34 | o | o | D35
                D36 | o | o | D32
                D39 | o | o | D33
                GND | o | o | D25
                D26 | o | o | D27
                D27 | o | o | D14
                D13 | o | o | D12
                D12 | o | o | GND
                GND | o | o | D23
                D23 | o | o | D22
                D22 | o | o | TX0
                RX0 | o | o | TX2
                    +---+---+

I2S Microphone (INMP441):
  CLK  → GPIO 26
  WS   → GPIO 25
  SD   → GPIO 33

I2S Speaker (MAX98357A):
  BCLK → GPIO 27
  LRCLK→ GPIO 14
  DIN  → GPIO 13

I2C Display (OLED SSD1306):
  SDA  → GPIO 21
  SCL  → GPIO 22

RGB LEDs (WS2812B):
  DIN  → GPIO 4
  GND  → GND
  VCC  → 5V (with level shifter on GPIO 4)

Servos (PWM):
  Servo 1 → GPIO 12
  Servo 2 → GPIO 13 (or other PWM pin)
  GND     → GND
  VCC     → 5V regulated

WiFi: Built-in (antenna on board)
MQTT: Via WiFi
```

## Installation

### 1. Install Arduino IDE 2

Download from: https://www.arduino.cc/software

### 2. Install ESP32 Board Support

1. Open Arduino IDE
2. Go to: File → Preferences
3. Add to "Additional Boards Manager URLs":
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Go to: Tools → Board Manager
5. Search "ESP32" and install "esp32" by Espressif Systems

### 3. Install Libraries

In Arduino IDE, go to: Sketch → Include Library → Manage Libraries

Required libraries:
- **PubSubClient** (MQTT) - by Nick O'Leary
- **Adafruit_SSD1306** (OLED) - by Adafruit
- **Adafruit_GFX** (Graphics) - by Adafruit
- **Adafruit_NeoPixel** (WS2812B LEDs) - by Adafruit
- **WiFiManager** - by tzapu

Search and install each one.

### 4. Configure Board Settings

In Arduino IDE:
- **Board**: ESP32 → ESP32 Dev Module
- **Upload Speed**: 921600
- **CPU Frequency**: 240 MHz
- **Flash Frequency**: 80 MHz
- **Flash Mode**: DIO
- **Flash Size**: 4MB (32Mb)
- **Partition Scheme**: Default 4MB with spiffs
- **Core Debug Level**: None

## Firmware Overview

The robot firmware (`robot.ino`) consists of several modules:

### Core Files

| File | Purpose |
|------|---------|
| `robot.ino` | Main program entry |
| `settings.h` | Configuration constants |
| `wifi_manager.cpp/h` | WiFi connection handling |
| `mqtt_handler.cpp/h` | MQTT communication |
| `speaker.cpp/h` | I2S speaker control |
| `inmp441.cpp/h` | I2S microphone |
| `servos.cpp/h` | Servo motor control |
| `rgb_led.h` | WS2812B LED control |
| `indicators.cpp/h` | LED status indicators |
| `oled_display.cpp/h` | OLED display |
| `roboeyes_display.cpp/h` | Animated eyes |

## Configuration

Edit `settings.h` to configure your robot:

```cpp
#ifndef SETTINGS_H
#define SETTINGS_H

// WiFi Configuration
#define WIFI_SSID "your-network-ssid"
#define WIFI_PASSWORD "your-network-password"

// MQTT Configuration
#define MQTT_SERVER "192.168.x.x"  // Your PC IP
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "robot_esp32"
#define MQTT_USER ""
#define MQTT_PASSWORD ""

// Audio Configuration
#define MIC_CLK_PIN 26
#define MIC_WS_PIN 25
#define MIC_DATA_PIN 33

#define SPEAKER_BCLK_PIN 27
#define SPEAKER_LRCLK_PIN 14
#define SPEAKER_DIN_PIN 13

// Display Configuration
#define OLED_SDA_PIN 21
#define OLED_SCL_PIN 22
#define OLED_ADDRESS 0x3C

// LED Configuration
#define LED_PIN 4
#define LED_COUNT 30

// Servo Configuration
#define SERVO1_PIN 12
#define SERVO2_PIN 13
#define SERVO_MIN_US 1000
#define SERVO_MAX_US 2000

#endif
```

## Uploading Firmware

### 1. Connect ESP32

Connect ESP32 to PC via USB cable. You should see a new COM port.

### 2. Select Port

In Arduino IDE:
- Tools → Port → COM[X] (ESP32)

### 3. Upload

1. Click Upload button (→ arrow icon)
2. Wait for compilation and upload
3. You should see "Hard resetting via RTS pin..." when complete

### 4. Monitor Serial Output

Tools → Serial Monitor (115200 baud)

You should see:
```
ESP32 Starting...
Setting up WiFi...
Connecting to WiFi...
WiFi connected! IP: 192.168.x.x
Connecting to MQTT...
MQTT connected!
```

## Firmware Features

### WiFi Connection

- Automatically connects to configured SSID
- If fails, creates AP for manual configuration
- IP address shown on OLED display

### MQTT Communication

**Subscribe Topics**:
- `robot/commands/execute` - Execute action
- `robot/commands/led` - Control LEDs
- `robot/commands/servo` - Move servos

**Publish Topics**:
- `robot/sensors/motion` - Motion detection
- `robot/status/online` - Online/offline status
- `robot/status/battery` - Battery level

### Audio Input/Output

- **Microphone**: Records audio stream via I2S
- **Speaker**: Plays audio via I2S MAX98357A
- Converts to/from WAV format for MQTT

### Display

- Shows IP address on startup
- Displays current robot state
- Shows received MQTT messages

### LED Indicators

- **Blue**: WiFi connected
- **Green**: MQTT connected
- **Red**: Error state
- **Rainbow**: Initialization

## Testing the Firmware

### Test 1: Serial Monitor

Upload and check serial output:
```
Serial Monitor should show connection progress
```

### Test 2: WiFi Connection

1. Check LED turns blue
2. Note IP address displayed
3. Verify you can ping: `ping 192.168.x.x`

### Test 3: MQTT Connection

1. Open MQTTX
2. Subscribe to: `robot/#`
3. ESP32 should publish status messages
4. LED should turn green

### Test 4: LED Test

Publish via MQTT:
```
Topic: robot/commands/led
Message: {"color":"red","brightness":255}
```

RGB LED should turn red.

### Test 5: Servo Test

Publish via MQTT:
```
Topic: robot/commands/servo
Message: {"servo":1,"angle":90}
```

Servo should move to 90 degrees.

## Troubleshooting

### Upload Fails

**Problem**: "Failed to open COM port" or "No board detected"

**Solutions**:
1. Check USB cable
2. Install CH340 driver: https://github.com/RobotDyn/CH340/releases
3. Restart Arduino IDE
4. Try different USB port

### No Serial Output

**Problem**: Serial monitor is blank

**Solutions**:
1. Check baud rate is 115200
2. Check correct COM port is selected
3. Restart ESP32 (press EN button)

### WiFi Connection Fails

**Problem**: "WiFi connection failed"

**Solutions**:
1. Check SSID and password in settings.h
2. Verify WiFi network is 2.4 GHz (ESP32 may not support 5 GHz)
3. Check signal strength

### MQTT Connection Fails

**Problem**: "MQTT connection failed"

**Solutions**:
1. Verify broker IP address
2. Check MQTT broker is running
3. Test with MQTTX on same network
4. Check firewall allows port 1883

### Audio Issues

**Problem**: No sound or noise

**Solutions**:
1. Check microphone wiring (especially I2S pins)
2. Verify speaker connections
3. Check speaker power (needs 5V)
4. Test with headphones to isolate issue

## Customization

### Add New Sensors

1. Create new `sensor.cpp/h` files
2. Initialize in setup()
3. Read data in loop()
4. Publish to MQTT

Example:

```cpp
#include "temperature_sensor.h"

void setup() {
  temp_sensor_init();
}

void loop() {
  float temp = read_temperature();
  
  char payload[32];
  snprintf(payload, sizeof(payload), "%.1f", temp);
  client.publish("robot/sensors/temperature", payload);
  
  delay(5000);
}
```

### Add New Actions

1. Add to MQTT callback handler
2. Implement action function
3. Publish status when complete

Example:

```cpp
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic, "robot/commands/dance") == 0) {
    execute_dance();
    client.publish("robot/status/action", "dance_complete");
  }
}

void execute_dance() {
  for (int i = 0; i < 4; i++) {
    servo_move(1, 90);
    delay(200);
    servo_move(1, 0);
    delay(200);
  }
}
```

## Next Steps

- Test all sensors and servos
- Integrate with MQTT broker
- Connect to N8N workflow
- Add custom behaviors

## Resources

- **ESP32 Documentation**: https://docs.espressif.com/projects/esp-idf/
- **Arduino IDE**: https://www.arduino.cc/
- **PubSubClient**: https://github.com/knolleary/pubsubclient
- **Adafruit Libraries**: https://github.com/adafruit

---

Last updated: December 2025
