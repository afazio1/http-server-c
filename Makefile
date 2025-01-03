.PHONY: build
build:
	docker build -t web_server .

run:
	docker run --rm -p 6969:6969 web_server 

# override default command and run with gdb
debug:
	docker run --rm -p 6969:6969 -it web_server gdb ./build/web_server

# Remove the Docker image
clean:
	docker rmi -f web_server
