# Docker EMQX Container Setup

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
