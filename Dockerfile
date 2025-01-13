# use an official image with a C compiler
FROM gcc:latest

# install additional tools like CMake and make
RUN apt-get update && apt-get install -y cmake make gdb

# set working directory inside the container
WORKDIR /http-server-c

# copy the project files into container
COPY . .

# expose the port for the server
EXPOSE 6969
