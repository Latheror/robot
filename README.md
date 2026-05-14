# 🤖 Margot - Intelligent Assistant Robot

An open-source AI-powered assistant robot project that brings together embedded systems, speech recognition, language models, and text-to-speech to create an interactive robot capable of listening, thinking, and responding.

## 🎯 Project Vision

**Margot** is designed to be:
- **Intelligent**: Connected to a large language model (LLM) for natural conversation and decision-making
- **Perceptive**: Equipped with microphones to listen and understand speech via Whisper AI
- **Interactive**: Can speak back using text-to-speech synthesis (Chatterbox TTS)
- **Connected**: Internet-enabled with real-time communication via MQTT
- **Autonomous**: Self-contained on an ESP32 microcontroller with integrated sensors and actuators

## 🏗️ System Architecture

The robot operates through a modular pipeline:

```
[Microphone Input] → [Speech Recognition] → [Decision Making] → [Action Execution]
    (INMP441)           (Whisper)            (LLM + N8N)      (LEDs, Servos, Speaker)
```

### Core Components

| Component | Technology | Role |
|-----------|-----------|------|
| **ESP32 Robot** | C++ Arduino | Reads sensors, controls actuators, communicates via MQTT |
| **Speech Recognition** | Whisper.cpp | Converts audio to text locally on device |
| **MQTT Broker** | EMQX | Message bus for robot-to-system communication |
| **Orchestration** | N8N Workflow | Connects signals → LLM → actions in a structured flow |
| **Language Model** | vLLM | Processes context and decides robot actions through an OpenAI-compatible API |
| **MCP Server** | Node.js REST API | Validates actions and ensures safe execution |
| **Text-to-Speech** | Chatterbox TTS | Generates natural speech responses in French |

### Data Flow

1. **Robot Senses** → ESP32 publishes sensor data (MQTT)
2. **System Listens** → N8N workflow receives the signal
3. **LLM Decides** → Queries vLLM with available capabilities
4. **Action Executes** → MCP validates and publishes command back to robot
5. **Robot Responds** → Executes action (movement, LED, speech)

## 🔐 MCP Pattern (Model Context Protocol)

For safety and extensibility, the system uses a **capability-driven pattern**:

- **GET /capabilities**: Robot announces what it can do in real time
- **LLM Decision**: Chooses from available actions only
- **POST /execute**: MCP validates and safely executes the action

**Benefits**: Security, consistency, extensibility, and fail-safe operation

## 📚 Project Structure

### 🔌 ESP32 Robot Code
**Location**: `ESP/robot/`

The embedded system running on the ESP32 microcontroller:
- **Sensors**: INMP441 microphone for audio input
- **Actuators**: Servos, RGB LED strips, OLED display (RoboEyes), speaker
- **Communication**: WiFi + MQTT for connection to the network
- **Functionality**: Sensor reading, motor control, audio capture, real-time feedback

**Key Files**:
- `robot.ino` - Main Arduino sketch
- `INMP441.cpp/h` - Microphone driver
- `servos.cpp/h` - Motor control
- `rgb_led.h` - LED animation
- `mqtt_handler.cpp/h` - MQTT communication
- `wifi_manager.cpp/h` - Network connectivity
- `roboeyes_display.cpp/h` - RoboEyes visual feedback
- `speaker.cpp/h` - Audio playback

### 🎤 Speech Recognition
**Location**: `SpeechToText/whisper/`

Local speech-to-text processing using OpenAI Whisper:
- Runs on the device for privacy and low latency
- Supports multiple languages
- Lightweight and efficient for embedded systems

**Setup Guide**: See `docs/guides/whisper-setup.md`

### 🧠 LLM & Orchestration
**Location**: `LLM/` and `N8N/`

**vLLM LLM**: OpenAI-compatible local language model inference server
- Processes context and makes decisions
- Selects appropriate robot actions

**N8N Workflow**: Visual automation connecting all components
- Listens for MQTT signals
- Queries MCP capabilities
- Calls LLM with context
- Executes validated actions
- Configuration: `N8N/n8n_workflow.json`

### 🔗 Message Broker (MQTT)
**Location**: `MQTT/` and `LLM/docker-compose.yml`

**EMQX**: Open-source MQTT broker
- Message bus between robot and system
- Enables publish/subscribe communication
- Dashboard at `localhost:18083`

**Setup**: See `docs/guides/mqtt-setup.md`

### 🗣️ Text-to-Speech
**Location**: `TextToSpeech/Chatterbox-TTS-Server/`

French text-to-speech synthesis using Chatterbox TTS:
- Generates natural-sounding speech
- Runs in Docker for easy deployment
- Integrated into robot responses

**Setup Guide**: See `docs/guides/tts-setup.md`

### 🏗️ Mechanical Design
**Location**: `Meca/3D/`

3D models and CAD files for:
- Robot head assembly
- INMP441 microphone mount
- OLED display enclosure
- Speaker housing

---

## 🚀 Quick Start

### Prerequisites
- ESP32 development board
- Python 3.8+
- Docker & Docker Compose
- Git (with submodules support)

### Installation Steps

1. **Clone the repository with submodules**
   ```bash
   git clone https://github.com/Latheror/robot.git
   cd robot
   git submodule update --init --recursive
   ```

2. **Set up infrastructure** (MQTT + LLM)
   ```bash
   # Start MQTT broker
   cd LLM
   docker-compose pull
   docker-compose up -d
   ```

3. **Flash ESP32**
   - Open `ESP/robot/robot.ino` in Arduino IDE
   - Install required libraries
   - Configure WiFi credentials in `settings.h`
   - Upload to board

4. **Configure and start services**
   - Import N8N workflow from `N8N/n8n_workflow.json`
   - Start vLLM for LLM inference
   - Start Chatterbox TTS server
   - See detailed guides in `docs/guides/`

---

## 📖 Documentation

Complete guides available in `docs/guides/`:

- [Getting Started](docs/guides/getting-started.md) - Project overview and setup
- [Hardware Setup](docs/guides/hardware.md) - Hardware components and assembly
- [ESP32 Robot Guide](docs/guides/esp32-robot.md) - Embedded system development
- [Whisper Speech Recognition](docs/guides/whisper-setup.md) - Speech-to-text setup
- [MQTT Configuration](docs/guides/mqtt-setup.md) - Message broker setup
- [LLM Setup](docs/guides/llm-setup.md) - Language model configuration
- [Text-to-Speech](docs/guides/tts-setup.md) - Voice synthesis setup
- [N8N Workflows](docs/guides/n8n-workflow.md) - Automation workflows
- [Architecture Deep Dive](docs/architecture/overview.md) - Detailed system design

---

## 🔧 Development

### Available Tasks (VS Code)

The project includes pre-configured tasks for development:

**ESP32 Development**
- `[ESP] Get PC IP Address` - Find your machine's IP for OTA uploads
- `[ESP] Erase Flash` - Clean the device memory

**Speech Recognition**
- `[WHISPER] Build Example` - Compile Whisper.cpp
- `[WHISPER] Build Binaries` - Build all binaries
- `[WHISPER] Test Sample` - Run sample audio test

**Text-to-Speech**
- `[CHATTERBOX] Generate example` - Test TTS output
- `[CHATTERBOX] Generate voice from text` - Synthesize custom text
- `[CHATTERBOX] Launch container (NVIDIA)` - GPU-accelerated TTS

**Infrastructure**
- `[LLM] Pull Images` - Download Docker images
- `[LLM] Start Services` - Launch MQTT and LLM
- `[LLM] Stop Services` - Stop services

---

## 📋 Infrastructure Setup

### Chapter: MQTT Broker with EMQX

EMQX is an open-source, scalable MQTT broker. Running it in Docker is the quickest way to start for development or testing.

#### Prerequisites

- [Docker](https://docs.docker.com/get-docker/) installed on your machine.

#### Steps

1. **Pull the EMQX Docker image**

   Open your terminal and run:
   ```
   docker pull emqx/emqx:latest
   ```

2. **Start the EMQX container**

   To start EMQX with standard ports:
   ```
   docker run -d --name emqx \
	 -p 1883:1883 \
	 -p 8083:8083 \
	 -p 8084:8084 \
	 -p 8883:8883 \
	 -p 18083:18083 \
	 emqx/emqx:latest
   ```

   - `-d`: detached mode
   - `--name emqx`: container name
   - `-p`: port mapping

3. **Access the EMQX dashboard**

   Open your browser at [http://localhost:18083](http://localhost:18083)
   Default credentials:
   - Username: `admin`
   - Password: `public`

4. **Stop and remove the container**

   To stop:
   ```
   docker stop emqx
   ```
   To remove:
   ```
   docker rm emqx
   ```

#### Notes

- For production, consider customizing the configuration and using Docker volumes.
- See the official documentation: [EMQX Docker](https://hub.docker.com/r/emqx/emqx)

### Chapter: Text-to-Speech with Chatterbox TTS Server

The project uses the [Chatterbox TTS Server](https://github.com/devnen/Chatterbox-TTS-Server) for text-to-speech functionality, configured for French language using the Thomcles/Chatterbox-TTS-French model.

The server is included as a git submodule in `TextToSpeech/Chatterbox-TTS-Server/`.

#### Running the Server

**CPU Version:**
```bash
cd TextToSpeech/Chatterbox-TTS-Server
docker-compose -f docker-compose-cpu.yml up --build
```

**NVIDIA GPU Version (faster):**
```bash
docker-compose -f docker-compose.yml up --build
```

The server will be available at `http://localhost:8004`

---

## 💡 How Everything Works Together

1. **Robot hears** something via the INMP441 microphone
2. **Whisper processes** the audio and converts it to text
3. **MQTT publishes** the transcribed text and sensor data
4. **N8N catches** the MQTT signal and queries MCP for available actions
5. **LLM decides** the best response based on context and available capabilities
6. **MCP validates** and safely executes the chosen action on the robot
7. **Robot responds** with movement, LED feedback, and speech synthesis

---

## 🤝 Contributing

This project is open source and contributions are welcome! Areas for improvement:
- Additional language support
- More sophisticated action templates
- Hardware optimizations
- 3D design improvements
- Documentation enhancements

---

## 📄 License

See individual component licenses in their respective directories.

---

## 📬 Contact & Support

For questions, issues, or suggestions, please open an issue on GitHub or check the documentation in `docs/`.

**Project Lead**: [Latheror](https://github.com/Latheror)
