# ESP32 Robot Control System

This project implements a comprehensive robot control system using an ESP32 microcontroller. The robot features servo control, audio input/output, visual indicators, OLED display, and MQTT-based communication for remote control and monitoring.

## Features

- **Servo Control**: 6-axis robotic arm control using PCA9685 PWM driver
- **Audio Processing**: INMP441 microphone for voice detection and audio capture
- **Audio Output**: Speaker system with WAV file playback from SPIFFS
- **Visual Feedback**: RGB LED indicators and NeoPixel LED strip
- **Display**: SH1106 OLED display with RoboEyes animation
- **Connectivity**: WiFi and MQTT for remote control and data streaming
- **I2C Scanning**: Automatic detection of connected I2C devices
- **Real-time Monitoring**: Heap usage monitoring and system diagnostics

## Hardware Requirements

- ESP32 development board (e.g., ESP32 Dev Module)
- PCA9685 16-channel PWM servo driver
- INMP441 microphone module (I2S interface)
- SH1106 OLED display (128x64, I2C)
- NeoPixel LED strip (5 LEDs)
- Speaker with I2S amplifier (e.g., MAX98357A)
- RGB LED
- Servo motors (6x, compatible with PCA9685)
- Connecting wires and power supply

### Pin Connections

| Component | ESP32 Pin |
|-----------|-----------|
| I2S SCK (Mic) | GPIO 4 |
| I2S WS (Mic) | GPIO 5 |
| I2S SD (Mic) | GPIO 6 |
| Voice Activity LED | GPIO 7 |
| I2C SDA | GPIO 17 |
| I2C SCL | GPIO 18 |
| LED Strip Data | GPIO 13 |

## Software Requirements

- Arduino IDE 1.8.19 or later
- ESP32 board support package
- Required Arduino libraries:
  - `Adafruit_PWMServoDriver`
  - `Adafruit_SH110X`
  - `Adafruit_GFX`
  - `Adafruit_NeoPixel`
  - `PubSubClient`
  - `ArduinoJson`
  - `LittleFS` (included with ESP32 core)

## Installation and Setup

1. **Install Arduino IDE and ESP32 Support**:
   - Download and install Arduino IDE
   - Add ESP32 board support via Board Manager (search for "esp32")

2. **Install Required Libraries**:
   - Open Arduino IDE
   - Go to Sketch > Include Library > Manage Libraries
   - Install each required library listed above

3. **Configure Project Settings**:
   - Open `settings.h`
   - Update WiFi credentials:
     ```cpp
     static constexpr const char* WIFI_SSID = "Your_WiFi_SSID";
     static constexpr const char* WIFI_PASSWORD = "Your_WiFi_Password";
     ```
   - Update MQTT broker settings:
     ```cpp
     static constexpr const char* MQTT_BROKER = "192.168.1.xxx"; // Your MQTT broker IP
     static constexpr int MQTT_PORT = 1883;
     ```

4. **Upload Audio Files**:
   - The `data/` folder contains WAV audio files
   - Use Arduino IDE's ESP32 Sketch Data Upload tool or LittleFS uploader
   - Audio files are played for system events (connection status, warnings, etc.)

5. **Select Board and Port**:
   - Board: "ESP32 Dev Module" or your specific ESP32 board
   - Port: Select the COM port where your ESP32 is connected

6. **Compile and Upload**:
   - Open `robot.ino` in Arduino IDE
   - Click "Verify" to compile
   - Click "Upload" to flash the firmware

## Usage

### Powering On
- Connect power to the ESP32
- The system will:
  - Initialize hardware components
  - Connect to WiFi
  - Establish MQTT connection
  - Start all FreeRTOS tasks

### MQTT Topics

The robot communicates via MQTT. Key topics:

- **Commands**: Send JSON commands to control servos, LEDs, etc.
- **Audio Stream**: Receive real-time audio data
- **Sensor Data**: Periodic system status updates
- **Status**: Connection and error notifications

### Serial Monitor
- Open Arduino IDE Serial Monitor (115200 baud)
- View debug output, connection status, and error messages

## Configuration

### Network Settings
Edit `settings.h` to configure:
- WiFi SSID and password
- MQTT broker IP and port
- Client ID

### Audio Settings
- Sampling rate: 16kHz
- Voice detection threshold: 10% of max volume
- Audio buffer sizes and processing parameters

### Hardware Pins
All pin assignments are defined in `settings.h`. Modify if your wiring differs.

### Task Timing
FreeRTOS task intervals are configurable in `TaskConfig` struct.

## Project Structure

- `robot.ino`: Main Arduino sketch
- `settings.h`: Configuration constants
- `wifi_manager.*`: WiFi connection handling
- `mqtt_handler.*`: MQTT communication
- `servos.*`: Servo motor control
- `oled_display.*`: OLED display management
- `roboeyes_display.*`: Animated eye display
- `speaker.*`: Audio output system
- `indicators.*`: LED indicators
- `INMP441.*`: Microphone input
- `led_strip.*`: NeoPixel LED strip
- `i2c_scanner.*`: I2C device detection
- `partitions.csv`: ESP32 flash partitioning
- `data/`: Audio files for SPIFFS

## Contributing

This project is open source under GPL v3. Contributions are welcome!

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE.txt](LICENSE.txt) file for details.
