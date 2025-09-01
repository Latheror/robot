# MQTTX Installation and Configuration

MQTTX is a cross-platform open-source MQTT client for testing and debugging MQTT communications.

## Installing MQTTX

There are several ways to install MQTTX:

### 1. Desktop Application Installation

1. Go to the official MQTTX website: https://mqttx.app/
2. Click on "Download"
3. Choose the version for your operating system:
   - Windows: `.exe`
   - MacOS: `.dmg`
   - Linux: `.AppImage` or `.deb`
4. Download and install the application

### 2. Web Installation (Online Version)

You can use MQTTX directly from your browser:
1. Visit: http://www.emqx.io/online-mqtt-client
2. No installation required

## Initial Setup

1. Launch MQTTX
2. To create a new connection, click the "+" button
3. Configure the basic parameters:
   - Name: Give your connection a name
   - Host: Your MQTT broker address (default: `localhost`)
   - Port: Connection port (default: `1883` for non-SSL, `8883` for SSL)
   - Username/Password: If required by your broker

## Connection Testing

1. After configuring the connection, click "Connect"
2. To test:
   - Click "New Subscription"
   - Enter a topic (e.g., "test/topic")
   - Send a test message to verify communication

## MQTT Broker Setup

We use EMQX as our MQTT broker. To set it up using Docker:

```bash
docker run -d --name robot_emqx -p 1883:1883 -p 8883:8883 -p 8083:8083 -p 8084:8084 -p 18083:18083 -e EMQX_NAME=robot_emqx -e EMQX_ALLOW_ANONYMOUS=true emqx/emqx-enterprise:5.10.0
```

This command:
- Creates a container named `robot_emqx`
- Exposes necessary ports:
  - 1883: Standard MQTT port
  - 8883: MQTT over SSL
  - 8083, 8084: WebSocket ports
  - 18083: Dashboard interface
- Enables anonymous connections
- Uses EMQX Enterprise 5.10.0

You can access:
- MQTT broker at localhost:1883
- Web dashboard at http://localhost:18083 (default credentials: admin/public)

## Important Notes

- Make sure Docker is installed and running
- For local connection in MQTTX, use `localhost` as host
- If connecting from another device, use your PC's IP address instead of localhost
