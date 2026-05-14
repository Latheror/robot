# System Architecture Overview

## High-Level Flow

The robot operates through an integrated pipeline:

```
Microphone → Whisper (STT) → N8N Workflow → vLLM (LLM) → MCP → MQTT → ESP32 → Robot Actions → Speaker
```

## Detailed Architecture

### 1. **Input Layer: Speech Recognition**

**Component**: Whisper.cpp (`Backend/whisper.cpp`)

- Captures audio from microphone or file
- Converts speech to text using OpenAI's Whisper model
- Runs locally for privacy
- Supports multiple languages
- Model: `ggml-small.bin` or larger

### 2. **Orchestration Layer: N8N Workflow**

**Component**: N8N (`N8N/n8n_workflow.json`)

The workflow handles the main pipeline:

```
1. MQTT Trigger
   └─ Listens for sensor signals from ESP32

2. Get Capabilities
   └─ HTTP GET /capabilities from MCP
   └─ Discovers available actions

3. Query LLM
   └─ Sends sensor data + capabilities to vLLM
   └─ LLM decides best action

4. Execute Action
   └─ HTTP POST /execute to MCP
   └─ MCP validates and publishes to MQTT
   └─ ESP32 receives and executes
```

### 3. **Decision Layer: LLM (vLLM)**

**Component**: vLLM (`LLM/docker-compose.yml`)

- Processes input context (sensor data, capabilities)
- Makes intelligent decisions
- Returns structured action: `{ action: string, parameters: object }`
- Runs in Docker container
- Configurable model (default served name: `robot-llm`)

### 4. **Communication Layer: MQTT**

**Component**: EMQX (`MQTT/Broker`)

- Pub/Sub messaging system
- Topics:
  - `robot/sensors/*` - Sensor data from ESP32
  - `robot/commands/*` - Commands to ESP32
  - `robot/status/*` - Robot status updates

**Broker**: EMQX 5.10.0
- Port: 1883 (MQTT)
- Dashboard: 18083 (Web UI)

### 5. **Control Layer: MCP (Model Context Protocol)**

**Purpose**: Central controller ensuring security and consistency

- Exposes capabilities dynamically
- Validates all actions before execution
- Acts as middleware between LLM and MQTT
- Ensures only authorized actions run

**Endpoints**:
- `GET /capabilities` - List available actions
- `POST /execute` - Execute validated action

### 6. **Hardware Layer: ESP32 Robot**

**Component**: ESP32 Firmware (`ESP/robot/robot.ino`)

Core modules:
- **INMP441**: I2S Microphone input
- **Speaker**: I2S Audio output
- **Servos**: Joint control (head, arms)
- **RGB LEDs**: Status indication
- **OLED Display**: Information display
- **RoboEyes**: Animated eye display
- **WiFi Manager**: Network connectivity
- **MQTT Handler**: Communication

**Firmware Responsibilities**:
- Connect to WiFi and MQTT broker
- Read sensor data (microphone)
- Publish sensor events
- Subscribe to command topics
- Execute received commands
- Update display and LEDs

### 7. **Output Layer: Text-to-Speech**

**Component**: Chatterbox-TTS (`TextToSpeech/Chatterbox-TTS-Server`)

- Converts text responses to speech
- Supports multiple languages (including French)
- Runs in Docker container
- High-quality voice synthesis

## Data Flow Sequence

```
┌─────────────────────────────────────────────────────────────┐
│ 1. USER SPEAKS TO MICROPHONE (ESP32)                        │
└────────────────────────┬────────────────────────────────────┘
                         │ MQTT: robot/audio/stream
                         │
┌────────────────────────▼────────────────────────────────────┐
│ 2. WHISPER TRANSCRIPTION (Backend)                          │
│    Speech → Text transcription                              │
└────────────────────────┬────────────────────────────────────┘
                         │ N8N receives text
                         │
┌────────────────────────▼────────────────────────────────────┐
│ 3. REQUEST CAPABILITIES (N8N → MCP)                         │
│    GET /capabilities                                        │
└────────────────────────┬────────────────────────────────────┘
                         │ Receive available actions
                         │
┌────────────────────────▼────────────────────────────────────┐
│ 4. LLM DECISION (vLLM)                                      │
│    Input: transcribed text + available capabilities         │
│    Output: { action: "...", parameters: {...} }            │
└────────────────────────┬────────────────────────────────────┘
                         │ N8N validates action
                         │
┌────────────────────────▼────────────────────────────────────┐
│ 5. EXECUTE ACTION (N8N → MCP)                               │
│    POST /execute with action & parameters                   │
└────────────────────────┬────────────────────────────────────┘
                         │ MCP publishes to MQTT
                         │
┌────────────────────────▼────────────────────────────────────┐
│ 6. MQTT PUBLISH (MCP → Broker)                              │
│    Topic: robot/commands/execute                            │
│    Payload: { action, parameters }                          │
└────────────────────────┬────────────────────────────────────┘
                         │ MQTT: robot/commands/execute
                         │
┌────────────────────────▼────────────────────────────────────┐
│ 7. ROBOT EXECUTION (ESP32)                                  │
│    - Subscribe to command topic                             │
│    - Execute action (move servo, show LED, etc.)            │
│    - Publish status back to MQTT                            │
└────────────────────────┬────────────────────────────────────┘
                         │ MQTT: robot/status/action_complete
                         │
┌────────────────────────▼────────────────────────────────────┐
│ 8. TEXT-TO-SPEECH RESPONSE (Chatterbox)                     │
│    Text → Audio generation                                  │
└────────────────────────┬────────────────────────────────────┘
                         │ Audio stream to ESP32 speaker
                         │
┌────────────────────────▼────────────────────────────────────┐
│ 9. ROBOT SPEAKS (Speaker Output)                            │
│    Response played through speaker                          │
└─────────────────────────────────────────────────────────────┘
```

## Technology Stack

| Layer | Technology | Purpose |
|-------|-----------|---------|
| **Speech Recognition** | Whisper.cpp | Local STT without external APIs |
| **Orchestration** | N8N | Visual workflow automation |
| **AI/LLM** | vLLM | Local OpenAI-compatible LLM inference |
| **Communication** | EMQX MQTT | Lightweight pub/sub messaging |
| **Hardware Control** | ESP32 | Microcontroller with wireless |
| **Text-to-Speech** | Chatterbox-TTS | Local voice synthesis |
| **Containerization** | Docker | Consistent deployments |

## Security Considerations

1. **MCP Validation**: All commands validated before MQTT publish
2. **Local Processing**: No data sent to external APIs
3. **MQTT Anonymous**: Configure authentication in production
4. **Network Isolation**: Keep robot and services on same network
5. **Firewall Rules**: Restrict MQTT access to trusted devices

## Performance Optimization

- **Whisper Model Size**: Use `base` or `small` for real-time transcription
- **LLM Model**: Use quantized models for faster inference
- **MQTT QoS**: Set appropriately (0 for speed, 1-2 for reliability)
- **WiFi**: Use 5GHz band for lower latency
- **Containers**: Run on capable hardware (4GB+ RAM recommended)

## Extension Points

The architecture allows easy integration:

1. **New Sensors**: Add MQTT publishers on ESP32
2. **New Actions**: Register in MCP capabilities endpoint
3. **New Commands**: LLM adapts to available capabilities
4. **Multiple Robots**: Add more ESP32 devices on same MQTT broker
5. **Cloud Integration**: Can replace local vLLM with any OpenAI-compatible LLM API

---

Last updated: December 2025
