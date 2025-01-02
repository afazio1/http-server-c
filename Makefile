.PHONY: build
build:
	docker build -t web_server .

run:
	docker run --rm web_server

# Remove the Docker image
clean:
	docker rmi -f web_server
