# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.0] - 2025-12-29

### SpeechToText
- Whisper.cpp integration for local speech-to-text
- CMake build system for Whisper compilation
- Multi-language support framework

### ESP32 Robot
- Core Arduino firmware for ESP32
- INMP441 microphone driver
- Servo motor control
- RGB LED strip support
- OLED display with RoboEyes
- Speaker audio playback
- WiFi manager for connectivity
- MQTT handler for communication

### LLM & Orchestration
- vLLM LLM server with Docker setup
- N8N workflow automation
- MCP Server pattern for safe action execution

### Text-to-Speech
- Chatterbox TTS Server for French synthesis
- Docker support (CPU and NVIDIA GPU versions)
- Git submodule integration

### Infrastructure
- MQTT Broker (EMQX) setup with Docker
- Docker Compose configuration for all services
- VS Code development tasks

### Mechanical Design
- 3D CAD models for robot assembly
- Component mounting designs (head, microphone, display, speaker)

### Documentation
- Complete setup guides for all components
- Architecture documentation
- Getting started guide
- Hardware setup guide
- GitHub Pages deployment
