# HTTP Server in C

## Build

```sh
make build
```

## Run
```sh
make run
```

## Run with GDB
```sh
make debug
```

## Connect to Server
```sh
nc localhost 6969
```

## Stop Container
```sh
docker ps # get running containers
docker stop <container_id> # send SIGTERM signal to docker container
```
