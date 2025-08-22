# Docker EMQX Container Setup
## Robot Project Overview

## General Architecture

ESP32 (robot)
- Simulates or reads sensors
- Publishes signals to MQTT (EMQX)
- Receives commands (actions/emotions) via MQTT and executes them (e.g., RoboEyes)

MCP Server (Node.js)
- Exposes a REST API:
	- GET /capabilities: lists possible actions and parameters
	- POST /execute: validates an action and publishes the command to MQTT
- Acts as the central controller (it actually drives the robot)

N8N Workflow
- Retrieves MQTT signals from the robot
- Requests capabilities from MCP
- Sends everything to the LLM (Ollama) for decision
- Posts the decision to MCP (/execute), which then sends it to the robot

LLM (Ollama)
- Processes received signals
- Chooses a valid action from those provided by MCP
- Returns JSON { action, parameters }

## MCP Pattern

Standard flow:
- GET /capabilities: discover what the robot can do in real time
- LLM decision: choose a valid action
- POST /execute: MCP checks and publishes the command to the robot

Advantages: security, consistency, extensibility

## N8N Workflow (Simplified)

- MQTT Trigger: waits for sensor signals
- HTTP Request (GET /capabilities): queries MCP
- HTTP Request (POST Ollama): asks the LLM what to do
- HTTP Request (POST /execute): sends the decision to MCP

👉 Result: The robot reacts to signals based on AI decisions, but MCP always retains final control.

## Chapter: Creating a Docker EMQX Container

EMQX is an open-source, scalable MQTT broker. Running it in a Docker container is a quick way to start for development or testing.

### Prerequisites

- [Docker](https://docs.docker.com/get-docker/) installed on your machine.

### Steps

1. **Pull the EMQX Docker image**

	Open your terminal and run:
	```
	docker pull emqx/emqx:latest
	```

2. **Start the EMQX container**

	To start EMQX with standard ports:
	```
	docker run -d --name emqx \
	  -p 1883:1883 \
	  -p 8083:8083 \
	  -p 8084:8084 \
	  -p 8883:8883 \
	  -p 18083:18083 \
	  emqx/emqx:latest
	```

	- `-d`: detached mode
	- `--name emqx`: container name
	- `-p`: port mapping

3. **Access the EMQX dashboard**

	Open your browser at [http://localhost:18083](http://localhost:18083)
	Default credentials:
	- Username: `admin`
	- Password: `public`

4. **Stop and remove the container**

	To stop:
	```
	docker stop emqx
	```
	To remove:
	```
	docker rm emqx
	```

### Notes

- For production, consider customizing the configuration and using Docker volumes.
- See the official documentation: [EMQX Docker](https://hub.docker.com/r/emqx/emqx)
