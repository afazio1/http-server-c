# HTTP Server in C

## Start Docker Container
```sh
./scripts/docker.sh --pull # Build docker image
./scripts/docker.sh # Run docker container
```
## Build

Note: may need to run `cmake` outside Docker container for `compile_commands.json` to work with LSP
```sh
cmake .. # generate build system
make # build + compile
```

## Run
```sh
./web_server
```

## Connect to Server
```sh
nc localhost 6969
```
