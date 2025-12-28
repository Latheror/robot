# LLM Setup Guide (Ollama)

## Overview

Ollama is an easy way to run large language models locally. We use it for the robot's decision-making capability - processing sensor data and capabilities to determine appropriate actions.

**Location**: `LLM/docker-compose.yml`

## Why Ollama?

- ✅ Runs entirely locally (no cloud API calls)
- ✅ Supports multiple models (Llama, Mistral, Dolphin, etc.)
- ✅ GPU acceleration support
- ✅ Simple API interface
- ✅ Easy model management

## Installation

### Option 1: Docker (Recommended)

```bash
docker run -d -v ollama:/root/.ollama -p 11434:11434 ollama/ollama
```

### Option 2: Using Docker Compose

Create `LLM/docker-compose.yml`:

```yaml
version: '3'
services:
  ollama:
    image: ollama/ollama
    container_name: ollama
    pull_policy: always
    tty: true
    restart: always
    environment:
      - OLLAMA_HOST=0.0.0.0:11434
    ports:
      - 11434:11434
    volumes:
      - ollama:/root/.ollama
    stdin_open: true
    # GPU support (uncomment for NVIDIA)
    # runtime: nvidia
    # environment:
    #   - NVIDIA_VISIBLE_DEVICES=all

volumes:
  ollama:
```

Run:
```bash
cd LLM
docker-compose up -d
```

### Option 3: Direct Installation

Visit: https://ollama.ai/download

## Downloading Models

### Pull a Model

```bash
# Via Docker
docker exec ollama ollama pull llama2

# Via Ollama CLI (if installed locally)
ollama pull llama2
```

### Available Models

| Model | Size | Speed | Capability |
|-------|------|-------|-----------|
| llama2 | 3.8 GB | Fast | Good reasoning |
| mistral | 4.1 GB | Fast | Very capable |
| neural-chat | 3.8 GB | Fast | Optimized for chat |
| dolphin-mixtral | 26 GB | Slow | Very capable |
| orca-mini | 1.3 GB | Very Fast | Quick decisions |

**Recommendation for Robot**: `llama2` or `neural-chat` for balance of speed and capability.

Pull multiple models:
```bash
docker exec ollama ollama pull llama2
docker exec ollama ollama pull mistral
docker exec ollama ollama pull neural-chat
```

## API Usage

### REST API Endpoint

Base URL: `http://localhost:11434`

### 1. Generate Text

**Endpoint**: `POST /api/generate`

```bash
curl http://localhost:11434/api/generate -d '{
  "model": "llama2",
  "prompt": "The capital of France is",
  "stream": false
}'
```

**Python**:
```python
import requests
import json

def query_ollama(prompt, model="llama2"):
    response = requests.post(
        "http://localhost:11434/api/generate",
        json={
            "model": model,
            "prompt": prompt,
            "stream": False
        }
    )
    return response.json()["response"]

# Usage
result = query_ollama("Tell me about robotics")
print(result)
```

**Node.js**:
```javascript
const axios = require('axios');

async function queryOllama(prompt, model = 'llama2') {
  const response = await axios.post('http://localhost:11434/api/generate', {
    model: model,
    prompt: prompt,
    stream: false
  });
  return response.data.response;
}

// Usage
queryOllama('Tell me about robotics').then(console.log);
```

### 2. Chat Mode

**Endpoint**: `POST /api/chat`

```bash
curl http://localhost:11434/api/chat -d '{
  "model": "llama2",
  "messages": [
    {"role": "user", "content": "Hello, how are you?"}
  ],
  "stream": false
}'
```

**Python**:
```python
def chat_with_ollama(messages, model="llama2"):
    response = requests.post(
        "http://localhost:11434/api/chat",
        json={
            "model": model,
            "messages": messages,
            "stream": False
        }
    )
    return response.json()["message"]["content"]

# Usage
messages = [
    {"role": "user", "content": "What should the robot do when it sees motion?"}
]
response = chat_with_ollama(messages)
print(response)
```

### 3. List Available Models

```bash
curl http://localhost:11434/api/tags
```

```python
def list_models():
    response = requests.get("http://localhost:11434/api/tags")
    return response.json()["models"]

models = list_models()
for model in models:
    print(f"{model['name']}: {model['size']}")
```

## Robot Decision-Making Integration

### Flow

```
Sensor Input + Available Capabilities → LLM → Action Decision
```

### Example: Motion Detection Response

**Sensor Data**:
```json
{
  "sensor": "motion",
  "event": "motion_detected",
  "confidence": 0.95
}
```

**Available Capabilities**:
```json
{
  "actions": [
    "greet_person",
    "blink_eyes",
    "play_sound",
    "move_head",
    "show_emotion"
  ],
  "emotions": ["happy", "curious", "neutral", "confused"]
}
```

**LLM Prompt**:
```
You are a friendly robot. Based on the following information, decide what action to take.

Current Sensor: Motion detected (95% confidence)
Available Actions: greet_person, blink_eyes, play_sound, move_head, show_emotion
Available Emotions: happy, curious, neutral, confused

Respond in JSON format:
{
  "action": "action_name",
  "parameters": {
    "key": "value"
  },
  "reasoning": "brief explanation"
}
```

**LLM Response**:
```json
{
  "action": "greet_person",
  "parameters": {
    "emotion": "happy",
    "gesture": "wave"
  },
  "reasoning": "Motion detected likely means someone is approaching, so greet them warmly"
}
```

### Python Integration Script

Create `robot_decision.py`:

```python
import requests
import json
import time

OLLAMA_URL = "http://localhost:11434"
OLLAMA_MODEL = "llama2"

def get_robot_decision(sensor_data, available_actions, available_emotions):
    """Get LLM decision based on sensor input"""
    
    prompt = f"""You are a friendly robot. Based on the following information, decide what action to take.

Current Sensor: {sensor_data['event']} (confidence: {sensor_data.get('confidence', 0.9)})
Available Actions: {', '.join(available_actions)}
Available Emotions: {', '.join(available_emotions)}

Respond ONLY in valid JSON format (no markdown, no extra text):
{{
  "action": "action_name",
  "parameters": {{}},
  "reasoning": "brief explanation"
}}

If no suitable action exists, use "neutral" emotion and "idle" action."""

    try:
        response = requests.post(
            f"{OLLAMA_URL}/api/generate",
            json={
                "model": OLLAMA_MODEL,
                "prompt": prompt,
                "stream": False,
                "temperature": 0.7
            },
            timeout=30
        )
        
        response_text = response.json()["response"].strip()
        
        # Extract JSON from response
        start = response_text.find("{")
        end = response_text.rfind("}") + 1
        json_str = response_text[start:end]
        
        decision = json.loads(json_str)
        return decision
        
    except Exception as e:
        print(f"Error querying LLM: {e}")
        return {"action": "idle", "parameters": {}, "reasoning": "Error occurred"}

# Example usage
if __name__ == "__main__":
    sensor_data = {
        "event": "motion_detected",
        "confidence": 0.95
    }
    
    actions = ["greet_person", "blink_eyes", "move_head", "show_emotion"]
    emotions = ["happy", "curious", "neutral", "confused"]
    
    decision = get_robot_decision(sensor_data, actions, emotions)
    print(json.dumps(decision, indent=2))
```

## Integration with N8N

In N8N workflow:

1. **HTTP Request Node** (to Ollama)
   - URL: `http://localhost:11434/api/generate`
   - Method: POST
   - Body:
     ```json
     {
       "model": "llama2",
       "prompt": "{{ $node.previousNode.json.prompt }}",
       "stream": false
     }
     ```

2. **Extract response**:
   ```
   {{ $node.OllamaRequest.json.response }}
   ```

3. **Parse as JSON** to extract action and parameters

## Performance Optimization

### 1. Keep Model in Memory

By default, models are unloaded after 5 minutes. To keep in memory:

```python
# Ping the model periodically
import requests
import time

def keep_model_loaded(model="llama2", interval=300):
    while True:
        requests.post(
            "http://localhost:11434/api/generate",
            json={"model": model, "prompt": "ok", "stream": False}
        )
        time.sleep(interval)

# Run in background thread
import threading
thread = threading.Thread(target=keep_model_loaded, daemon=True)
thread.start()
```

### 2. Use Smaller Model for Fast Responses

For time-critical decisions:
```bash
docker exec ollama ollama pull orca-mini  # 1.3 GB, very fast
```

### 3. GPU Acceleration

For NVIDIA GPUs:

```bash
docker run -d --gpus all -v ollama:/root/.ollama -p 11434:11434 ollama/ollama
```

For AMD GPUs:
```bash
docker run -d --device=/dev/kfd --device=/dev/dri -v ollama:/root/.ollama -p 11434:11434 ollama/ollama
```

## Troubleshooting

### Connection Refused

**Problem**: `Connection refused` when accessing `localhost:11434`

**Solutions**:
1. Check container is running: `docker ps | grep ollama`
2. Verify port: `docker logs ollama`
3. Restart: `docker restart ollama`

### Model Not Found

**Problem**: `Error: model not found` when using model

**Solutions**:
1. Pull the model: `docker exec ollama ollama pull llama2`
2. List available: `curl http://localhost:11434/api/tags`

### Slow Responses

**Problem**: LLM responses take too long

**Solutions**:
1. Use faster model: `ollama pull orca-mini`
2. Enable GPU acceleration
3. Reduce prompt length
4. Use streaming for long responses

### High Memory Usage

**Problem**: Container using too much RAM

**Solutions**:
1. Use smaller model (orca-mini instead of llama2)
2. Unload model to free memory: `docker exec ollama ollama serve`
3. Limit memory: `docker run -m 4g ...`

## Advanced: Custom Models

### Fine-tune a Model

```bash
# Create custom model from llama2
ollama create custom-robot -f Modelfile
```

**Modelfile**:
```
FROM llama2
SYSTEM You are a helpful robot assistant focused on robotics and automation.
PARAMETER temperature 0.7
PARAMETER top_p 0.9
```

## Next Steps

- Integrate with N8N workflow
- Connect to Whisper for STT processing
- Set up response generation for TTS

## Resources

- **Ollama**: https://ollama.ai
- **Models Available**: https://ollama.ai/library
- **API Docs**: https://github.com/ollama/ollama/blob/main/docs/api.md

---

Last updated: December 2025
