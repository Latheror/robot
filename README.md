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

EMQX est un broker MQTT open-source et scalable. L'exécuter dans un conteneur Docker est une méthode rapide pour démarrer en développement ou test.

### Prérequis

- [Docker](https://docs.docker.com/get-docker/) installé sur votre machine.

### Étapes

1. **Récupérer l'image Docker EMQX**

	Ouvrez votre terminal et lancez :
	```
	docker pull emqx/emqx:latest
	```

2. **Démarrer le conteneur EMQX**

	Pour lancer EMQX avec les ports standards :
	```
	docker run -d --name emqx \
	  -p 1883:1883 \
	  -p 8083:8083 \
	  -p 8084:8084 \
	  -p 8883:8883 \
	  -p 18083:18083 \
	  emqx/emqx:latest
	```

	- `-d` : mode détaché
	- `--name emqx` : nom du conteneur
	- `-p` : mappage des ports

3. **Accéder au dashboard EMQX**

	Ouvrez votre navigateur sur [http://localhost:18083](http://localhost:18083)
	Identifiants par défaut :
	- Utilisateur : `admin`
	- Mot de passe : `public`

4. **Arrêter et supprimer le conteneur**

	Pour arrêter :
	```
	docker stop emqx
	```
	Pour supprimer :
	```
	docker rm emqx
	```

### Notes

- Pour la production, pensez à personnaliser la configuration et à utiliser des volumes Docker.
- Voir la documentation officielle : [EMQX Docker](https://hub.docker.com/r/emqx/emqx)
