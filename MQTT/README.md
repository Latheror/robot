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

## Important Notes

- Make sure you have an MQTT broker (like EMQX, Mosquitto) running
- For local connection, use `localhost` as host
- Standard ports are:
  - 1883: Unsecured MQTT
  - 8883: Secured MQTT (SSL/TLS)
