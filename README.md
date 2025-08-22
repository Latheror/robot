# Docker EMQX Container Setup

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
