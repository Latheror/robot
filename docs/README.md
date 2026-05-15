# Robot Project Documentation

Welcome to the Robot project documentation! This guide covers all aspects of the project from architecture to component-specific setup.

## Quick Navigation

- **[Architecture Overview](./architecture/overview.md)** - System design and component interactions
- **[Getting Started](./guides/getting-started.md)** - Initial setup and prerequisites
- **[MQTT Setup](./guides/mqtt-setup.md)** - Configuring the MQTT broker (EMQX)
- **[ESP32 Robot](./guides/esp32-robot.md)** - Hardware setup and firmware
- **[Speech Recognition (Whisper)](./guides/whisper-setup.md)** - Audio transcription backend
- **[Text-to-Speech (Chatterbox)](./guides/tts-setup.md)** - Voice generation
- **[LLM Integration (vLLM)](./guides/llm-setup.md)** - AI decision-making
- **[N8N Workflow](./guides/n8n-workflow.md)** - Automation pipeline
- **[Hardware (Meca)](./guides/hardware.md)** - 3D models and mechanical design

## Project Overview

This is an intelligent robot project that combines:
- **Audio Input**: Speech recognition via Whisper.cpp
- **AI Decision Making**: Large language models (vLLM)
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
│    SpeechToText/whisper + SpeechToText/whisper-api          │
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
│ vLLM (LLM)        │  │ MQTT Broker       │
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
| **Whisper.cpp** | `SpeechToText/whisper/` | Speech-to-text transcription |
| **Whisper API** | `SpeechToText/whisper-api/` | HTTP wrapper around Whisper for workflows |
| **ESP32 Robot** | `ESP/robot/` | Robot firmware and hardware control |
| **MQTT Broker** | `MQTT/` | Message broker for robot communication |
| **LLM** | `LLM/` | vLLM container for AI decisions |
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
├── ESP/                   # ESP32 firmware and hardware code
├── LLM/                   # Large Language Model (vLLM)
├── MQTT/                  # MQTT broker configuration
├── N8N/                   # Workflow automation
├── SpeechToText/          # Whisper.cpp and Whisper HTTP API
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
- **vLLM**: https://docs.vllm.ai
- **N8N**: https://n8n.io
- **EMQX MQTT**: https://www.emqx.io
- **Chatterbox TTS**: https://github.com/devnen/Chatterbox-TTS-Server

---

Last updated: May 2026
