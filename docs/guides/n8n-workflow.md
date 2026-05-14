# N8N Workflow Setup

## Overview

N8N is a visual workflow automation platform that orchestrates the entire robot pipeline. It connects speech recognition, LLM decision-making, and robot action execution.

**Location**: `N8N/n8n_workflow.json`

## Workflow Architecture

```
MQTT Trigger
    ↓
Audio Stream (Whisper.cpp)
    ↓
Get Capabilities (HTTP → MCP)
    ↓
Query LLM (OpenAI-compatible HTTP → vLLM)
    ↓
Parse Decision
    ↓
Execute Action (HTTP → MCP)
    ↓
Publish to MQTT
    ↓
ESP32 Receives & Executes
```

## Installation

### Option 1: Docker (Recommended)

```bash
docker run -d --name n8n -p 5678:5678 n8nio/n8n
```

Access at: http://localhost:5678

### Option 2: Docker Compose

Create `docker-compose.yml`:

```yaml
version: '3'
services:
  n8n:
    image: n8nio/n8n
    container_name: n8n
    ports:
      - "5678:5678"
    environment:
      - N8N_BASIC_AUTH_ACTIVE=false
      - N8N_HOST=0.0.0.0
      - N8N_PORT=5678
      - N8N_PROTOCOL=http
      - NODE_ENV=production
    volumes:
      - n8n_data:/home/node/.n8n
    restart: always

volumes:
  n8n_data:
```

Run: `docker-compose up -d`

### Option 3: Direct Installation

Download from: https://n8n.io/download

## Workflow Components

### 1. MQTT Trigger

**Purpose**: Listen for sensor events from ESP32

**Configuration**:
- **MQTT Host**: `192.168.x.x` (your PC IP)
- **Port**: 1883
- **Topic**: `robot/sensors/#`
- **Username/Password**: Leave empty (anonymous)

**Output**: Receives sensor data

```json
{
  "topic": "robot/sensors/motion",
  "message": "motion_detected",
  "timestamp": 1234567890
}
```

### 2. Whisper Transcription

**Purpose**: Convert audio to text (if microphone input)

**Option A: Execute Whisper Binary**

```bash
whisper-cli -m models/ggml-base.en.bin -f {{ $node.MQTT_Input.json.audio_file }} -ojson
```

**Option B: Call Whisper API**

```
POST http://localhost:8000/transcribe
Body: { "audio": "base64_encoded_audio" }
```

**Output**: Transcribed text

### 3. Get Capabilities

**Purpose**: Fetch available robot actions

**Configuration**:
- **Method**: GET
- **URL**: `http://localhost:8000/capabilities` (MCP server)

**Response**:
```json
{
  "actions": ["greet_person", "blink_eyes", "move_head", "show_emotion"],
  "emotions": ["happy", "curious", "neutral", "confused"],
  "parameters": {
    "greet_person": ["gesture", "emotion"],
    "move_head": ["angle", "duration"]
  }
}
```

### 4. Query LLM

**Purpose**: Get AI decision based on input and capabilities

**Configuration**:
- **Method**: POST
- **URL**: `http://host.docker.internal:8001/v1/chat/completions`
- **Body**:

```json
{
  "model": "robot-llm",
  "messages": [
    {"role": "system", "content": "You are a friendly robot. Decide only from available actions."},
    {"role": "user", "content": "Sensor Input: {{ $node.MQTT_Input.json.message }}\nAvailable Actions: {{ $node.GetCapabilities.json.actions }}"}
  ],
  "max_tokens": 120,
  "temperature": 0.2
}
```

**Output**:
```json
{
  "action": "greet_person",
  "parameters": {
    "emotion": "happy",
    "gesture": "wave"
  }
}
```

### 5. Execute Action

**Purpose**: Validate and execute the decided action

**Configuration**:
- **Method**: POST
- **URL**: `http://localhost:8000/execute` (MCP server)
- **Body**:

```json
{
  "action": "{{ $node.LLM_Decision.json.action }}",
  "parameters": "{{ $node.LLM_Decision.json.parameters }}"
}
```

**Response**: Action acknowledgment

### 6. Publish to MQTT

**Purpose**: Send execution command to ESP32

**Configuration**:
- **MQTT Host**: `192.168.x.x`
- **Topic**: `robot/commands/execute`
- **Message**:

```json
{
  "action": "{{ $node.ExecuteAction.json.action }}",
  "parameters": "{{ $node.ExecuteAction.json.parameters }}",
  "timestamp": "{{ $now.toIso() }}"
}
```

### 7. Generate Response (TTS)

**Purpose**: Convert LLM text response to speech

**Configuration**:
- **Method**: POST
- **URL**: `http://localhost:8000/tts`
- **Body**:

```json
{
  "text": "{{ $node.LLM_Decision.json.response_text }}",
  "language": "en",
  "speaker": "en-us-glow-tts"
}
```

**Output**: Audio file (WAV)

### 8. Publish Audio to MQTT

**Purpose**: Send response audio to ESP32

**Configuration**:
- **Topic**: `robot/audio/response`
- **Message**: Binary audio data

## Import Workflow

### From File

1. Open N8N dashboard
2. Click "Import" (top right)
3. Select `N8N/n8n_workflow.json`
4. Click "Import"

### Manual Setup

If importing doesn't work, manually recreate:

1. Create new workflow
2. Add MQTT Trigger node
3. Connect to Whisper Execute node
4. Connect to HTTP Request (Get Capabilities)
5. Connect to HTTP Request (Query LLM)
6. Connect to HTTP Request (Execute Action)
7. Connect to MQTT Publish node
8. Connect to HTTP Request (TTS)
9. Connect to MQTT Publish (audio)

## Testing the Workflow

### Test 1: MQTT Trigger

1. Open MQTTX
2. Publish to: `robot/sensors/motion`
3. Message: `motion_detected`
4. Check N8N logs (should show execution)

### Test 2: Each Node Individually

1. Select node
2. Click "Test" button
3. Check output is as expected
4. Debug using node output

### Test 3: Full Workflow

1. Ensure all services running:
   - MQTT Broker
  - vLLM LLM
   - Chatterbox TTS
   - MCP Server
   - ESP32

2. Trigger via MQTT:
   ```
   Topic: robot/sensors/motion
   Message: motion_detected
   ```

3. Check N8N execution
4. Verify ESP32 receives command

### Test 4: Performance

Track execution time:
1. Enable "Execution History"
2. Monitor timing for each node
3. Optimize slow nodes

## Workflow Configuration

### Environment Variables

Set via N8N settings or docker environment:

```env
MQTT_HOST=localhost
MQTT_PORT=1883
LLM_URL=http://localhost:8001/v1
WHISPER_URL=http://localhost:8000
TTS_URL=http://localhost:8000
MCP_URL=http://localhost:3000
ESP32_IP=192.168.x.x
```

### Error Handling

Add error handlers to critical nodes:

1. **MQTT Connection Fails**
   - Retry 3 times
   - Wait 5 seconds between retries
   - Send alert notification

2. **LLM Timeout**
   - Set 30 second timeout
   - Fall back to default action
   - Log for debugging

3. **MQTT Publish Fails**
   - Retry publish
   - Queue message for later
   - Send to monitoring system

### Advanced: Caching Decisions

For repeated inputs, cache decisions:

```javascript
// Execute (JavaScript) Node
const cacheKey = `${msg.topic}_${msg.message}`;
const cached = await $node.GetCached.json.decisions[cacheKey];

if (cached) {
  return cached;  // Use cached decision
} else {
  // Get new decision from LLM
  return newDecision;
}
```

## Monitoring & Logging

### Enable Logging

In N8N settings:
1. Set log level to DEBUG
2. View logs: `docker logs n8n`

### Monitor Workflow Execution

1. N8N Dashboard → Executions
2. View each execution
3. Check input/output of each node
4. Debug using "Execute Node" button

### Alert on Failure

Add notification nodes:

```
Error Handler
    ↓
Send Email/Slack/Discord
    ↓
Log to Database
```

## Customization

### Add New Sensor Trigger

1. Duplicate MQTT Trigger node
2. Change topic: `robot/sensors/temperature`
3. Add custom handling

### Add New Action

1. Update MCP capabilities endpoint
2. Add to the vLLM system prompt
3. Add handler in ESP32 firmware
4. Test in N8N workflow

### Change LLM Model

In "Query LLM" node:
```json
{
  "model": "robot-llm"
}
```

## Performance Tips

1. **Use smaller LLM model** (orca-mini) for speed
2. **Cache common decisions** to reduce API calls
3. **Batch MQTT messages** instead of processing individually
4. **Use webhooks** instead of polling for better performance
5. **Monitor execution time** and optimize bottlenecks

## Troubleshooting

### Workflow Doesn't Trigger

**Problem**: MQTT Trigger not receiving messages

**Solutions**:
1. Check MQTT broker is running
2. Verify topic matches exactly
3. Verify ESP32 is publishing
4. Check N8N has MQTT credentials
5. Review N8N logs

### MQTT Connection Error

**Problem**: `Connection refused` in MQTT node

**Solutions**:
1. Check MQTT host/port
2. Verify broker is running: `docker ps | grep emqx`
3. Check firewall allows 1883
4. Restart N8N container

### LLM Not Responding

**Problem**: HTTP timeout to vLLM

**Solutions**:
1. Check vLLM is running and healthy
2. Verify URL is correct
3. Increase timeout value
4. Check LLM model is loaded

### Slow Execution

**Problem**: Workflow takes too long

**Solutions**:
1. Use faster LLM model
2. Reduce number of nodes
3. Cache decisions
4. Parallel process where possible
5. Monitor each node timing

## Resources

- **N8N Documentation**: https://docs.n8n.io
- **N8N Community**: https://community.n8n.io
- **Workflow Examples**: https://n8n.io/workflows

---

Last updated: December 2025
