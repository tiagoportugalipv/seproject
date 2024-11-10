mkdir {data,log}
podman run -d -p 1883:1883 -v ./config:/mosquitto/config -v ./data:/mosquitto/data -v ./log:/mosquitto/log --name mqtt-server eclipse-mosquitto
