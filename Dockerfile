# use an official image with a C compiler
FROM gcc:latest

# install additional tools like CMake and make
RUN apt-get update && apt-get install -y cmake make gdb

# set working directory inside the container
WORKDIR /app

# copy the project files into container
COPY . .

# build the project with cmake
RUN mkdir -p build && cd build && cmake .. && make

# command to run the compiled executable
CMD ["./build/web_server"]


