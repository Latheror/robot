# Whisper.cpp Speech Recognition Setup

## Overview

Whisper is OpenAI's speech-to-text (STT) model that runs locally without requiring external APIs. Whisper.cpp is an optimized C++ implementation for fast, CPU-friendly inference.

**Location**: `Backend/whisper.cpp/`

## Features

- ✅ No internet required (local processing)
- ✅ Supports 99 languages
- ✅ Runs on CPU (GPU optional)
- ✅ Accurate transcription
- ✅ Multiple model sizes (tiny, base, small, medium, large)

## Model Selection

| Model | Size | Speed | Accuracy | RAM |
|-------|------|-------|----------|-----|
| tiny | 75 MB | Very Fast | Lower | 1 GB |
| base | 141 MB | Fast | Good | 2 GB |
| small | 466 MB | Medium | Better | 3 GB |
| medium | 1.5 GB | Slow | Very Good | 5 GB |
| large | 2.9 GB | Very Slow | Best | 10 GB |

**Recommendation**: Use `base` or `small` for real-time robot interaction.

## Installation

### 1. Prerequisites

- CMake 3.13 or later
- C++ compiler (MSVC on Windows, GCC on Linux)
- Git

### 2. Clone Repository

```bash
cd Backend
git clone https://github.com/ggml-org/whisper.cpp.git
cd whisper.cpp
```

### 3. Download Model

Download a model from Hugging Face:

```bash
# Base model (recommended for robot)
bash models/download-ggml-model.sh base.en

# Or small model for better accuracy
bash models/download-ggml-model.sh small.en

# For other languages, use the language code:
bash models/download-ggml-model.sh base.fr  # French
```

Model will be saved in `models/` directory.

### 4. Build

**Windows (MSVC)**:
```bash
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
```

**Linux/macOS**:
```bash
cmake -B build
cmake --build build -j4
```

### 5. Verify Installation

```bash
# Test with sample audio
./build/bin/whisper-cli -m models/ggml-base.en.bin -f samples/jfk.wav
```

Should output transcribed text.

## Usage

### Command Line

```bash
# Basic transcription
./build/bin/whisper-cli -m models/ggml-base.en.bin -f audio.wav

# With options
./build/bin/whisper-cli \
  -m models/ggml-base.en.bin \
  -f audio.wav \
  --language en \
  --output-txt \
  --output-vtt
```

### From Python

```python
import subprocess
import json

def transcribe_audio(audio_file, model_path="Backend/whisper.cpp/models/ggml-base.en.bin"):
    cmd = [
        "Backend/whisper.cpp/build/bin/whisper-cli",
        "-m", model_path,
        "-f", audio_file,
        "-ojson"
    ]
    
    result = subprocess.run(cmd, capture_output=True, text=True)
    output = json.loads(result.stdout)
    return output["result"][0]["text"]

# Usage
text = transcribe_audio("input.wav")
print(f"Transcribed: {text}")
```

### From Node.js

Use the `whisper.cpp/bindings/javascript/` bindings:

```javascript
const Whisper = require('./bindings/javascript/whisper.js');

const whisper = new Whisper({
  modelPath: 'models/ggml-base.en.bin'
});

const result = whisper.transcribe('audio.wav');
console.log('Transcribed:', result.text);
```

## Integration with N8N

In N8N workflow:

1. **Add HTTP Request node**
   - Method: POST
   - URL: `http://localhost:8000/transcribe` (custom endpoint)
   - Body: `{ "audio": "<base64-encoded-audio>" }`

2. **Or use Execute node**
   - Execute command: `whisper-cli -m models/ggml-base.en.bin -f audio.wav -ojson`
   - Parse JSON output

3. **Or call Python script**
   ```javascript
   // In N8N Execute (Python) node
   import subprocess
   audio_file = "{{$node['previous'].json.audio}}"
   result = subprocess.run([
       "python", "scripts/transcribe.py", audio_file
   ], capture_output=True, text=True)
   return json.loads(result.stdout)
   ```

## Performance Optimization

### 1. GPU Acceleration

**NVIDIA CUDA**:
```bash
cmake -B build -DWHISPER_CUDA=ON
cmake --build build --config Release
```

**Apple Metal**:
```bash
cmake -B build -DWHISPER_METAL=ON
cmake --build build --config Release
```

### 2. Use Smaller Model

Start with `base.en` instead of `small.en` for faster processing.

### 3. Reduce Audio Length

Process in chunks instead of long files:
- Chunk: 30 seconds of audio
- Parallel processing: Multiple chunks simultaneously

### 4. Pre-allocate GPU Memory

```bash
# Set GPU memory for faster repeated calls
export CUDA_LAUNCH_BLOCKING=0
```

## Audio Input Methods

### From File

```bash
whisper-cli -m model.bin -f audio.wav
```

### From Microphone (Real-time)

Use FFmpeg to capture microphone:

```bash
# Windows
ffmpeg -f dshow -i "audio=\"Microphone\"" -ac 1 -ar 16000 - | whisper-cli -m model.bin -f -

# Linux
ffmpeg -f alsa -i default -ac 1 -ar 16000 - | whisper-cli -m model.bin -f -

# macOS
ffmpeg -f avfoundation -i ":0" -ac 1 -ar 16000 - | whisper-cli -m model.bin -f -
```

### From MQTT Stream

1. ESP32 publishes audio chunks to MQTT
2. N8N collects chunks
3. Reconstruct audio file
4. Pass to Whisper

## Supported Languages

Whisper supports 99 languages including:
- English (en)
- French (fr)
- Spanish (es)
- German (de)
- Chinese (zh)
- Japanese (ja)
- And many more...

Download language-specific models:
```bash
bash models/download-ggml-model.sh base.fr  # French
bash models/download-ggml-model.sh base.de  # German
```

## Troubleshooting

### Model Download Fails

**Problem**: Slow or failing download

**Solutions**:
1. Use alternative mirror: Edit `download-ggml-model.sh`
2. Manual download: Visit https://huggingface.co/ggerganov/whisper.cpp/tree/main
3. Use smaller model first (tiny or base)

### Slow Transcription

**Problem**: Taking too long to process

**Solutions**:
1. Use smaller model (tiny or base)
2. Enable GPU acceleration (CUDA/Metal)
3. Reduce audio length
4. Check CPU usage: Increase available cores if possible

### Out of Memory

**Problem**: `Out of memory` error

**Solutions**:
1. Use tiny or base model (not medium/large)
2. Process shorter audio chunks
3. Close other applications
4. Enable swap space

### Inaccurate Results

**Problem**: Transcription quality is poor

**Solutions**:
1. Use larger model (small or medium)
2. Improve audio quality (reduce noise)
3. Specify correct language
4. Check sample rate is 16kHz

## Advanced: Running as Service

### Docker Container

Create `Dockerfile`:

```dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    cmake build-essential git ffmpeg \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN cd Backend/whisper.cpp && \
    cmake -B build && \
    cmake --build build -j4

EXPOSE 8000

CMD ["python", "transcribe_server.py"]
```

Build and run:
```bash
docker build -t whisper-service .
docker run -p 8000:8000 whisper-service
```

### Python Flask API

Create `transcribe_server.py`:

```python
from flask import Flask, request, jsonify
import subprocess
import json
import os

app = Flask(__name__)

WHISPER_BIN = "Backend/whisper.cpp/build/bin/whisper-cli"
MODEL = "Backend/whisper.cpp/models/ggml-base.en.bin"

@app.route('/transcribe', methods=['POST'])
def transcribe():
    if 'audio' not in request.files:
        return jsonify({'error': 'No audio file'}), 400
    
    audio_file = request.files['audio']
    audio_path = f"/tmp/{audio_file.filename}"
    audio_file.save(audio_path)
    
    try:
        result = subprocess.run(
            [WHISPER_BIN, "-m", MODEL, "-f", audio_path, "-ojson"],
            capture_output=True,
            text=True
        )
        output = json.loads(result.stdout)
        text = output["result"][0]["text"]
        return jsonify({'text': text})
    finally:
        os.remove(audio_path)

if __name__ == '__main__':
    app.run(port=8000, debug=False)
```

Run:
```bash
python transcribe_server.py
```

## Next Steps

- Integrate with N8N workflow
- Connect to ESP32 for real-time audio
- Set up Chatterbox TTS for response

## Resources

- **GitHub**: https://github.com/ggml-org/whisper.cpp
- **Models**: https://huggingface.co/ggerganov/whisper.cpp
- **Paper**: https://arxiv.org/abs/2212.04356

---

Last updated: December 2025
