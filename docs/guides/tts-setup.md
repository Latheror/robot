# Text-to-Speech (Chatterbox) Setup Guide

## Overview

Chatterbox-TTS is a high-quality text-to-speech (TTS) engine that converts text responses into natural-sounding speech. We use a French-customized fork for multilingual support.

**Location**: `TextToSpeech/Chatterbox-TTS-Server/`  
**Repository**: https://github.com/Latheror/chatterbox-tts-server-french

## Features

- ✅ High-quality voice synthesis
- ✅ Multiple language support (including French)
- ✅ Various speaker voices
- ✅ Real-time processing
- ✅ REST API interface
- ✅ GPU acceleration support

## Installation

### Option 1: Docker (Recommended)

```bash
cd TextToSpeech/Chatterbox-TTS-Server
docker-compose up -d
```

### Option 2: Docker Compose with GPU

For NVIDIA GPU acceleration:

```bash
docker-compose -f docker-compose.yml up -d
```

The compose file automatically uses GPU if available.

### Option 3: Manual Installation

**Prerequisites**:
- Python 3.8+
- PyTorch
- FFmpeg

**Steps**:

```bash
cd TextToSpeech/Chatterbox-TTS-Server

# Create virtual environment
python -m venv venv
source venv/bin/activate  # or venv\Scripts\activate on Windows

# Install dependencies
pip install -r requirements.txt

# Download models (first run only)
python download_model.py

# Start server
python server.py
```

## API Usage

### REST Endpoint

**Base URL**: `http://localhost:8000`

### 1. Generate Speech

**Endpoint**: `POST /tts`

```bash
curl -X POST http://localhost:8000/tts \
  -H "Content-Type: application/json" \
  -d '{
    "text": "Hello, I am Margot the robot!",
    "language": "en",
    "speaker": "en-us-glow-tts"
  }' \
  --output output.wav
```

**Parameters**:
- `text` (required): Text to convert to speech
- `language` (optional): Language code (en, fr, de, es, etc.)
- `speaker` (optional): Speaker/voice identifier
- `speed` (optional): Speech speed (0.5 to 2.0, default 1.0)

### 2. List Available Speakers

```bash
curl http://localhost:8000/speakers
```

**Response**:
```json
{
  "speakers": [
    "en-us-glow-tts",
    "en-gb-glow-tts",
    "fr-fr-glow-tts",
    "de-de-glow-tts"
  ]
}
```

### 3. Generate with Options

```bash
curl -X POST http://localhost:8000/tts \
  -H "Content-Type: application/json" \
  -d '{
    "text": "Bonjour, je suis Margot!",
    "language": "fr",
    "speaker": "fr-fr-glow-tts",
    "speed": 1.0,
    "pitch": 1.0
  }' \
  --output output.wav
```

## Language & Speaker Support

### Supported Languages

| Language | Code | Default Speaker |
|----------|------|------------------|
| English | en | en-us-glow-tts |
| French | fr | fr-fr-glow-tts |
| German | de | de-de-glow-tts |
| Spanish | es | es-es-glow-tts |
| Italian | it | it-it-glow-tts |
| Portuguese | pt | pt-pt-glow-tts |
| Dutch | nl | nl-nl-glow-tts |

### Available Speakers

Each language typically has multiple speaker options. List available speakers:

```bash
curl http://localhost:8000/speakers | jq
```

## Python Integration

### Basic Usage

```python
import requests
from io import BytesIO

def text_to_speech(text, language="en", speaker="en-us-glow-tts", output_file=None):
    """Convert text to speech"""
    
    response = requests.post(
        "http://localhost:8000/tts",
        json={
            "text": text,
            "language": language,
            "speaker": speaker
        }
    )
    
    if response.status_code == 200:
        if output_file:
            with open(output_file, 'wb') as f:
                f.write(response.content)
            print(f"Saved to {output_file}")
        return response.content
    else:
        raise Exception(f"Error: {response.text}")

# Example usage
audio = text_to_speech(
    "Hello, I am a robot!",
    language="en",
    speaker="en-us-glow-tts",
    output_file="response.wav"
)
```

### Advanced: Play Audio Directly

```python
import pyaudio
import numpy as np
from scipy.io import wavfile
from io import BytesIO

def speak(text, language="en", speaker="en-us-glow-tts"):
    """Convert text to speech and play immediately"""
    
    response = requests.post(
        "http://localhost:8000/tts",
        json={
            "text": text,
            "language": language,
            "speaker": speaker
        }
    )
    
    # Parse WAV data
    audio_data = BytesIO(response.content)
    sample_rate, data = wavfile.read(audio_data)
    
    # Play audio
    p = pyaudio.PyAudio()
    stream = p.open(
        format=pyaudio.paFloat32,
        channels=1,
        rate=sample_rate,
        output=True
    )
    
    stream.write(data.astype(np.float32).tobytes())
    stream.stop_stream()
    stream.close()
    p.terminate()

# Usage
speak("Bonjour, comment allez-vous?", language="fr", speaker="fr-fr-glow-tts")
```

## Node.js Integration

```javascript
const axios = require('axios');
const fs = require('fs');

async function textToSpeech(text, options = {}) {
  const {
    language = 'en',
    speaker = 'en-us-glow-tts',
    outputFile = null
  } = options;

  try {
    const response = await axios.post('http://localhost:8000/tts', {
      text,
      language,
      speaker
    }, {
      responseType: 'arraybuffer'
    });

    if (outputFile) {
      fs.writeFileSync(outputFile, response.data);
      console.log(`Saved to ${outputFile}`);
    }

    return response.data;
  } catch (error) {
    console.error('TTS Error:', error.message);
    throw error;
  }
}

// Usage
textToSpeech('Hello robot', {
  language: 'en',
  speaker: 'en-us-glow-tts',
  outputFile: 'output.wav'
});
```

## Integration with N8N Workflow

In N8N workflow:

1. **HTTP Request Node**
   - URL: `http://localhost:8000/tts`
   - Method: POST
   - Content Type: application/json
   - Body:
     ```json
     {
       "text": "{{ $node.LLMResponse.json.response }}",
       "language": "en",
       "speaker": "en-us-glow-tts"
     }
     ```

2. **Save File Node** (optional)
   - Save audio output to file

3. **Send to ESP32**
   - Publish to MQTT topic: `robot/audio/response`
   - ESP32 receives and plays through speaker

## Integration with ESP32

### Arduino Code

```cpp
#include <WiFiClient.h>
#include <PubSubClient.h>

void play_tts_response(const char* text) {
  // Call Chatterbox TTS server
  HTTPClient http;
  
  String payload = String("{\"text\":\"") + text + 
                   "\",\"language\":\"en\",\"speaker\":\"en-us-glow-tts\"}";
  
  http.begin("http://192.168.x.x:8000/tts");
  http.addHeader("Content-Type", "application/json");
  
  int httpCode = http.POST(payload);
  
  if (httpCode == 200) {
    // Get audio stream
    WiFiClient* stream = http.getStreamPtr();
    
    // Write to I2S (speaker)
    play_audio_stream(stream);
  }
  
  http.end();
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  String message = String((char*)payload).substring(0, length);
  
  if (String(topic) == "robot/tts/speak") {
    play_tts_response(message.c_str());
  }
}
```

## Performance Optimization

### 1. Use GPU Acceleration

The docker-compose file automatically uses GPU if available. Check:

```bash
docker logs chatterbox-tts-server | grep -i gpu
```

### 2. Batch Processing

For multiple TTS requests:

```python
def batch_tts(texts, language="en", speaker="en-us-glow-tts"):
    """Convert multiple texts efficiently"""
    
    results = []
    for text in texts:
        audio = text_to_speech(text, language, speaker)
        results.append(audio)
    
    return results

# Use case: Pre-generate common phrases
common_phrases = [
    "Hello!",
    "How can I help?",
    "Thank you!",
    "Goodbye!"
]

cached_responses = batch_tts(common_phrases)
```

### 3. Cache Common Responses

```python
import hashlib
from pathlib import Path

CACHE_DIR = Path("tts_cache")
CACHE_DIR.mkdir(exist_ok=True)

def text_to_speech_cached(text, language="en", speaker="en-us-glow-tts"):
    """TTS with caching to avoid repeated requests"""
    
    # Create cache key
    cache_key = hashlib.md5(f"{text}_{language}_{speaker}".encode()).hexdigest()
    cache_file = CACHE_DIR / f"{cache_key}.wav"
    
    # Return cached if exists
    if cache_file.exists():
        return cache_file.read_bytes()
    
    # Otherwise, request and cache
    audio = text_to_speech(text, language, speaker)
    cache_file.write_bytes(audio)
    
    return audio
```

## Troubleshooting

### Connection Refused

**Problem**: `Connection refused` at `localhost:8000`

**Solutions**:
1. Check container running: `docker ps | grep chatterbox`
2. Check logs: `docker logs chatterbox-tts-server`
3. Restart: `docker restart chatterbox-tts-server`

### Model Download Fails

**Problem**: Error downloading models

**Solutions**:
1. Check internet connection
2. Manually download: `python download_model.py`
3. Check disk space (needs ~2GB)

### Slow Response

**Problem**: TTS generation takes too long

**Solutions**:
1. Enable GPU acceleration
2. Use smaller models
3. Check CPU/memory usage

### Out of Memory

**Problem**: `CUDA out of memory` or similar

**Solutions**:
1. Reduce batch size
2. Use CPU only: Remove GPU config
3. Increase available memory

## Advanced: Custom Models

You can use different TTS models:

1. **Glow-TTS** (default, high quality)
2. **Tacotron2** (older, but good)
3. **FastPitch** (very fast)

Configure in `config.yaml`:

```yaml
model:
  name: "glow-tts"  # Change to tacotron2 or fastpitch
  language: "en"
```

## Next Steps

- Integrate with N8N workflow
- Connect to ESP32 speaker output
- Set up voice caching for performance

## Resources

- **Chatterbox GitHub**: https://github.com/Latheror/chatterbox-tts-server-french
- **Original Project**: https://github.com/devnen/Chatterbox-TTS-Server
- **TTS Models**: https://github.com/coqui-ai/TTS

---

Last updated: December 2025
