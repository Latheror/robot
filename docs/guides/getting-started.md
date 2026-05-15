# Getting Started Guide

## Prerequisites

Before setting up the Robot project, ensure you have the following installed:

### Required Software

- **Git** - Version control: https://git-scm.com
- **Docker & Docker Compose** - Container platform: https://www.docker.com/products/docker-desktop
- **Python 3.8+** - Required for dependencies
- **Node.js 16+** (optional) - For MCP server development
- **Arduino IDE 2** (for ESP32 flashing)

### Hardware (for physical robot)

- **ESP32 Development Board** (e.g., ESP-WROOM-32)
- **INMP441 Microphone Module** (I2S)
- **MAX98357A Audio Amplifier** (I2S)
- **Servo Motors** (for robot movement)
- **1.3" OLED Display** (128x64, I2C)
- **WS2812B RGB LEDs** (addressable)
- **Power supply** (5V, adequate current)

## Installation Steps

### 1. Clone the Repository

```bash
git clone https://github.com/Latheror/robot.git
cd robot
```

### 2. Initialize Submodules

The project includes external components such as Whisper.cpp and Chatterbox-TTS-Server. Initialize submodules to ensure nested dependencies are present:

```bash
git submodule update --init --recursive
```

### 3. Create Python Virtual Environment

```bash
# Windows
python -m venv .venv
.venv\Scripts\activate

# macOS/Linux
python3 -m venv .venv
source .venv/bin/activate
```

### 4. Install Python Dependencies

```bash
python -m pip install --upgrade pip setuptools wheel
python -m pip install numpy torch soundfile huggingface-hub safetensors
```

## Quick Start with Docker

The easiest way to get started is using Docker Compose:

### Start MQTT Broker (EMQX)

```bash
docker run -d --name robot_emqx \
  -p 1883:1883 \
  -p 18083:18083 \
  emqx/emqx:latest
```

Access dashboard: http://localhost:18083 (admin/public)

### Start vLLM LLM

```bash
docker compose -f LLM/docker-compose.yml pull
docker compose -f LLM/docker-compose.yml up -d
```

### Start Chatterbox TTS Server

```bash
cd TextToSpeech/Chatterbox-TTS-Server
docker-compose up -d
```

## Component-Specific Setup

Once docker services are running, refer to component guides:

1. **[MQTT Setup](./mqtt-setup.md)** - Configure broker and client
2. **[Whisper Setup](./whisper-setup.md)** - Speech recognition
3. **[LLM Setup](./llm-setup.md)** - vLLM configuration
4. **[TTS Setup](./tts-setup.md)** - Text-to-speech
5. **[ESP32 Setup](./esp32-robot.md)** - Hardware firmware
6. **[N8N Setup](./n8n-workflow.md)** - Workflow automation

## Verify Installation

### Check Docker Services

```bash
docker ps
```

Should show running containers:
- `robot_emqx` - MQTT broker
- `vllm` - LLM service
- `chatterbox-tts-server` - TTS service
- `whisper-stt` - optional STT HTTP API if you use `SpeechToText/whisper-api`

### Check Python Environment

```bash
python --version  # Should be 3.8+
pip list          # Should show installed packages
```

### Test MQTT Connection

```bash
# Using Docker to test
docker run --rm emqx/mqttx-cli sub -h host.docker.internal -t "robot/test"
```

## Environment Variables

Create a `.env` file in the project root for configuration:

```env
# MQTT Configuration
MQTT_HOST=localhost
MQTT_PORT=1883
MQTT_USER=admin
MQTT_PASSWORD=public

# vLLM LLM
VLLM_BASE_URL=http://localhost:8001/v1
VLLM_MODEL=robot-llm

# Chatterbox TTS
TTS_HOST=http://localhost:8000
TTS_SPEAKER=en-us

# ESP32 WiFi (configure in ESP firmware)
WIFI_SSID=your-network
WIFI_PASSWORD=your-password
```

## Troubleshooting

### Docker Won't Start
- Ensure Docker Desktop is running
- Check available disk space (Docker containers need ~20GB)
- Verify Docker socket permissions

### MQTT Connection Failed
- Check EMQX is running: `docker ps | grep emqx`
- Verify ports: `localhost:1883` for MQTT, `localhost:18083` for dashboard
- Check firewall rules

### Whisper Very Slow
- Larger models are slower (base is faster than medium)
- Check CPU/GPU availability
- Consider using smaller model: `ggml-base.en.bin`

### ESP32 Upload Fails
- Ensure correct COM port selected in Arduino IDE
- Install CH340 drivers if needed
- Check USB cable quality

## Next Steps

1. Read the **[Architecture Overview](../architecture/overview.md)**
2. Configure each component following component guides
3. Start with a simple test (e.g., publish MQTT message)
4. Build up to full voice interaction pipeline
5. Customize for your needs

## Getting Help

- Check component-specific README files
- Review logs in `docker logs <container-name>`
- Check GitHub issues: https://github.com/Latheror/robot/issues

---

Last updated: May 2026
