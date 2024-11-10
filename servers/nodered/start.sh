mkdir data
podman run -d -it -p 1880:1880 -v data:/data --name nodered nodered/node-red
