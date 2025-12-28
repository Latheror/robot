# Robot Project Documentation

Welcome to the Robot project documentation! This guide covers all aspects of the project from architecture to component-specific setup.

## Quick Navigation

- **[Architecture Overview](./architecture/overview.md)** - System design and component interactions
- **[Getting Started](./guides/getting-started.md)** - Initial setup and prerequisites
- **[MQTT Setup](./guides/mqtt-setup.md)** - Configuring the MQTT broker (EMQX)
- **[ESP32 Robot](./guides/esp32-robot.md)** - Hardware setup and firmware
- **[Speech Recognition (Whisper)](./guides/whisper-setup.md)** - Audio transcription backend
- **[Text-to-Speech (Chatterbox)](./guides/tts-setup.md)** - Voice generation
- **[LLM Integration (Ollama)](./guides/llm-setup.md)** - AI decision-making
- **[N8N Workflow](./guides/n8n-workflow.md)** - Automation pipeline
- **[Hardware (Meca)](./guides/hardware.md)** - 3D models and mechanical design

## Project Overview

This is an intelligent robot project that combines:
- **Audio Input**: Speech recognition via Whisper.cpp
- **AI Decision Making**: Large language models (Ollama)
- **Audio Output**: Text-to-speech via Chatterbox-TTS
- **Robot Control**: ESP32-based hardware with MQTT communication
- **Automation**: N8N workflows orchestrating the entire system

The robot can listen to voice commands, understand them with AI, decide on appropriate actions, and respond with speech while executing physical commands.

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     Voice Input (Microphone)                │
└────────────────────┬────────────────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────────────────┐
│        Whisper.cpp (Speech Recognition Backend)             │
│              TextToSpeech/Backend/whisper.cpp               │
└────────────────────┬────────────────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────────────────┐
│            N8N Workflow Orchestration                        │
│              LLM/docker-compose.yml                         │
└────────────────────┬────────────────────────────────────────┘
                     │
         ┌───────────┴───────────┐
         │                       │
┌────────▼──────────┐  ┌────────▼──────────┐
│ Ollama (LLM)      │  │ MQTT Broker       │
│ LLM Decision      │  │ (EMQX)            │
└────────┬──────────┘  │ Communication     │
         │             └────────┬──────────┘
         │                      │
         └──────────┬───────────┘
                    │
         ┌──────────▼──────────┐
         │ ESP32 Robot Hardware│
         │  - Sensors          │
         │  - LEDs/Display     │
         │  - Servos           │
         └──────────┬──────────┘
                    │
         ┌──────────▼──────────┐
         │ Text-to-Speech      │
         │ (Chatterbox)        │
         └──────────┬──────────┘
                    │
         ┌──────────▼──────────┐
         │  Voice Output       │
         │  (Speaker)          │
         └─────────────────────┘
```

## Key Components

| Component | Location | Purpose |
|-----------|----------|---------|
| **Whisper.cpp** | `Backend/whisper.cpp` | Speech-to-text transcription |
| **ESP32 Robot** | `ESP/robot/` | Robot firmware and hardware control |
| **MQTT Broker** | `MQTT/Broker` | Message broker for robot communication |
| **LLM** | `LLM/` | Ollama container for AI decisions |
| **Chatterbox TTS** | `TextToSpeech/Chatterbox-TTS-Server` | Text-to-speech engine |
| **N8N** | `N8N/` | Workflow automation |
| **Hardware** | `Meca/3D/` | 3D models and mechanical designs |

## Getting Started

1. Start with **[Getting Started Guide](./guides/getting-started.md)** for prerequisites and initial setup
2. Follow component-specific guides for detailed configuration
3. Refer to **[Architecture Overview](./architecture/overview.md)** for understanding system flow

## Project Structure

```
Robot/
├── Backend/               # Speech recognition (Whisper.cpp)
├── ESP/                   # ESP32 firmware and hardware code
├── LLM/                   # Large Language Model (Ollama)
├── MQTT/                  # MQTT broker configuration
├── N8N/                   # Workflow automation
├── TextToSpeech/          # Text-to-speech (Chatterbox)
├── Meca/                  # Mechanical design and 3D models
└── docs/                  # This documentation
```

## Contributing

When making changes, please:
1. Update relevant documentation
2. Commit changes with clear messages
3. Test thoroughly before pushing

## Support & Resources

- **Whisper.cpp**: https://github.com/ggml-org/whisper.cpp
- **Ollama**: https://ollama.ai
- **N8N**: https://n8n.io
- **EMQX MQTT**: https://www.emqx.io
- **Chatterbox TTS**: https://github.com/devnen/Chatterbox-TTS-Server

---

Last updated: December 2025
