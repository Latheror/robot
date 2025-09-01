# MQTT Setup Guide

This guide explains how to set up and use MQTT for the robot project.

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

The broker will be available at:
- MQTT: localhost:1883
- Dashboard: http://localhost:18083 (credentials: admin/public)

## MQTTX Installation

MQTTX is a user-friendly MQTT client for testing and debugging. To install:

1. Go to https://mqttx.app/
2. Download the appropriate version:
   - Windows: `.exe`
   - MacOS: `.dmg`
   - Linux: `.AppImage` or `.deb`

Alternatively, use the web version at http://www.emqx.io/online-mqtt-client

## Connecting to the Broker

1. Launch MQTTX
2. Create a new connection:
   - Click "+"
   - Name: `Robot_MQTT` (or any name)
   - Host: Select "Other" from dropdown, then type either:
     - `127.0.0.1` (recommended)
     - or `localhost` (if 127.0.0.1 doesn't work)
   - Port: `1883`
   - Client ID: Leave as auto-generated
   - Leave username/password empty
   - SSL/TLS: Off

See `monitoring_client/broker_connection_config.png` for a visual reference of these settings.

## Subscribing to Robot Data

1. After connecting, click "New Subscription"
2. Enter these settings:
   - Topic: `robot/1/sensors`
   - QoS: 0

See `monitoring_client/topic_subscription_config.png` for a visual reference of the subscription setup.

You'll receive messages every 2 seconds in this format:
```json
{
  "timestamp": 123456789,
  "sensors": {
    "temperature": 22.75,
    "humidity": 45.5,
    "light": 850
  },
  "units": {
    "temperature": "celsius",
    "humidity": "percent",
    "light": "lux"
  }
}
```

## Important Notes

- Ensure Docker is running before starting the broker
- Use `localhost` if MQTTX is on the same PC as the broker
- Use the PC's IP address if connecting from another device
