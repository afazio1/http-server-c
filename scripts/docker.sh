#!/usr/bin/env bash

ROOT_DIR="$(pwd)"

PULL=false
ATTACH=false

for arg in "$@"; do
  case $arg in
    --pull)
      PULL=true
      shift 
      ;;
    --attach)
      ATTACH=true
      shift 
      ;;
    *)
      echo "invalid flag"
      ;;
  esac
done

if [[ $PULL == true ]]; then
  echo "Building image"
  docker build -t web_server .
elif [[ $ATTACH == true ]]; then
  echo "Attaching to container"
  docker exec -it http-server-c bash 
else
  echo "starting http server in c container"
  docker run --rm -it --name="http-server-c" -v "${ROOT_DIR}/":/http-server-c -w="/http-server-c" -p 6969:6969 web_server
fi
