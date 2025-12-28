# MQTT Setup Guide

## Overview

MQTT is the communication backbone of the robot project. We use **EMQX** as our MQTT broker - an open-source, highly scalable MQTT message broker.

## MQTT Broker Setup (EMQX)

### Option 1: Docker (Recommended)

Start EMQX container with all required ports:

```bash
docker run -d --name robot_emqx \
  -p 1883:1883 \
  -p 8883:8883 \
  -p 8083:8083 \
  -p 8084:8084 \
  -p 18083:18083 \
  -e EMQX_ALLOW_ANONYMOUS=true \
  emqx/emqx:latest
```

**Port Mapping**:
- `1883` - Standard MQTT protocol
- `8883` - MQTT over SSL/TLS
- `8083` - WebSocket
- `8084` - WebSocket over SSL
- `18083` - Web Dashboard

**Dashboard Access**:
- URL: http://localhost:18083
- Default credentials: `admin` / `public`

### Option 2: Docker Compose

Create `docker-compose.yml`:

```yaml
version: '3.8'
services:
  emqx:
    image: emqx/emqx:latest
    container_name: robot_emqx
    environment:
      EMQX_ALLOW_ANONYMOUS: "true"
      EMQX_DASHBOARD_DEFAULT_USER: admin
      EMQX_DASHBOARD_DEFAULT_PASSWORD: public
    ports:
      - "1883:1883"
      - "8883:8883"
      - "8083:8083"
      - "8084:8084"
      - "18083:18083"
    volumes:
      - emqx_data:/opt/emqx/data
      - emqx_log:/opt/emqx/log

volumes:
  emqx_data:
  emqx_log:
```

Then run:
```bash
docker-compose up -d
```

### Option 3: Local Installation

Download from: https://www.emqx.io/try

Or install via package manager (requires native installation).

## Stop and Remove EMQX

```bash
# Stop container
docker stop robot_emqx

# Remove container
docker rm robot_emqx

# Remove volume (data)
docker volume rm <volume_name>
```

## MQTT Client Setup

### MQTTX Desktop Application

MQTTX is a user-friendly MQTT client for testing and debugging.

**Installation**:
1. Download from https://mqttx.app/
2. Install for your OS (Windows .exe, macOS .dmg, Linux .AppImage)

**Configuration**:
1. Launch MQTTX
2. Click "+" to create new connection
3. Configure:
   - **Name**: `Robot` (or your preference)
   - **Protocol**: MQTT
   - **Host**: `127.0.0.1` or `localhost`
   - **Port**: `1883`
   - **Client ID**: Leave as auto-generated
   - **Username/Password**: Leave empty (anonymous)
   - **SSL/TLS**: OFF

4. Click "Connect"

### MQTTX Web Version

No installation needed:
- URL: http://www.emqx.io/online-mqtt-client
- Same configuration as desktop version

### Command Line (MQTT-CLI)

```bash
# Subscribe to a topic
mqtt-cli sub -h localhost -p 1883 -t "robot/sensors/#"

# Publish a message
mqtt-cli pub -h localhost -p 1883 -t "robot/commands/test" -m "Hello"
```

## MQTT Topics

The robot project uses the following MQTT topic structure:

### Sensor Topics (ESP32 → MQTT → N8N)

```
robot/sensors/microphone    - Audio stream or trigger
robot/sensors/motion        - Motion detection events
robot/sensors/temperature   - Temperature readings
robot/sensors/humidity      - Humidity readings
robot/status/battery        - Battery level
```

### Command Topics (N8N → MQTT → ESP32)

```
robot/commands/execute      - Action execution
robot/commands/led          - LED control
robot/commands/servo        - Servo movement
robot/commands/display      - OLED display control
```

### Status Topics (ESP32 → MQTT)

```
robot/status/online         - Connection status
robot/status/action_done    - Action completion
robot/status/emotion        - Current emotion/state
```

## MQTT Connection Configuration

### Arduino ESP32 Code

Example connection code for ESP32:

```cpp
#include <PubSubClient.h>
#include <WiFi.h>

// MQTT Configuration
const char* mqtt_server = "192.168.x.x";    // Your PC IP
const int mqtt_port = 1883;
const char* mqtt_client_id = "robot_esp32";
const char* mqtt_user = "";                 // Empty for anonymous
const char* mqtt_pass = "";                 // Empty for anonymous

WiFiClient espClient;
PubSubClient client(espClient);

void setup_mqtt() {
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqtt_callback);
  
  // Connect to MQTT
  while (!client.connected()) {
    if (client.connect(mqtt_client_id, mqtt_user, mqtt_pass)) {
      Serial.println("MQTT connected");
      // Subscribe to command topics
      client.subscribe("robot/commands/#");
    } else {
      delay(5000);
    }
  }
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  // Handle received message
  String message = String((char*)payload).substring(0, length);
  Serial.print("MQTT received: ");
  Serial.println(message);
}

void loop() {
  if (!client.connected()) {
    setup_mqtt();
  }
  client.loop();
  
  // Publish sensor data
  if (/* condition */) {
    client.publish("robot/sensors/motion", "detected");
  }
}
```

### Python/Node.js Application

**Python Example**:

```python
import paho.mqtt.client as mqtt

MQTT_BROKER = "localhost"
MQTT_PORT = 1883

def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code {rc}")
    client.subscribe("robot/commands/#")

def on_message(client, userdata, msg):
    print(f"Received: {msg.topic} -> {msg.payload.decode()}")

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(MQTT_BROKER, MQTT_PORT, 60)
client.loop_start()

# Publish a message
client.publish("robot/sensors/test", "hello")

# Keep running
try:
    import time
    while True:
        time.sleep(1)
except KeyboardInterrupt:
    client.loop_stop()
```

**Node.js Example**:

```javascript
const mqtt = require('mqtt');

const client = mqtt.connect('mqtt://localhost:1883', {
  clientId: 'robot-node'
});

client.on('connect', () => {
  console.log('MQTT connected');
  client.subscribe('robot/commands/#', (err) => {
    if (err) console.error(err);
  });
});

client.on('message', (topic, message) => {
  console.log(`Received: ${topic} -> ${message.toString()}`);
});

// Publish a message
client.publish('robot/sensors/test', 'hello', () => {
  console.log('Message published');
});
```

## Testing MQTT Connection

### Test 1: Check Broker Status

```bash
# Using MQTTX Web or Desktop
# Connect and you should see "Connected" status
```

### Test 2: Publish Test Message

Using MQTTX:
1. Connect to broker
2. In "Publish" section:
   - Topic: `robot/test`
   - Message: `hello`
   - Click "Publish"

### Test 3: Subscribe and Receive

Using MQTTX:
1. Click "+" to add subscription
2. Topic: `robot/test`
3. You should see your published message

## MQTT Security (Production)

For production deployments:

### 1. Enable Authentication

In EMQX dashboard:
1. Go to Access Control → Users
2. Change default password
3. Add specific users for ESP32, N8N, etc.

### 2. Enable TLS/SSL

```bash
docker run -d --name robot_emqx \
  -p 1883:1883 \
  -p 8883:8883 \
  -v /path/to/certs:/opt/emqx/etc/certs \
  emqx/emqx:latest
```

### 3. Network Isolation

- Restrict MQTT broker to internal network only
- Use firewall rules to block external access
- Consider VPN for remote connections

## Troubleshooting

### Connection Refused

**Problem**: `Connection refused` when connecting to `localhost:1883`

**Solutions**:
1. Check EMQX is running: `docker ps | grep emqx`
2. Verify port: `netstat -an | findstr 1883` (Windows)
3. Restart Docker: `docker restart robot_emqx`

### Connection Timeout

**Problem**: Connection attempt hangs

**Solutions**:
1. Verify IP address is correct
2. Check firewall allows port 1883
3. Test with `localhost` instead of IP address

### Anonymous Connection Denied

**Problem**: `Error: Not authorized` despite `EMQX_ALLOW_ANONYMOUS=true`

**Solutions**:
1. Restart container: `docker restart robot_emqx`
2. Check environment variable is set correctly
3. Clear client cache (MQTTX: Settings → Clear Data)

### High Latency

**Problem**: Message delays are too high

**Solutions**:
1. Use local broker (localhost) instead of remote
2. Reduce publishing frequency
3. Set QoS to 0 for speed (trade-off: no guarantee)

## Next Steps

- Configure N8N MQTT connection (see [N8N Guide](./n8n-workflow.md))
- Set up ESP32 MQTT client (see [ESP32 Guide](./esp32-robot.md))
- Review MQTT message format in N8N workflow

## Resources

- **EMQX Documentation**: https://docs.emqx.com/
- **MQTT Specification**: https://mqtt.org/
- **MQTTX Documentation**: https://mqttx.app/docs
- **Paho MQTT (Python)**: https://www.eclipse.org/paho/index.php?page=clients/python/index.php

---

Last updated: December 2025
