#include "stdio.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include "string.h"
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include "handle_client.h"

#define QUEUE_SIZE 20
#define MY_PORT "6969"

int sockfd;

void addr_to_str(struct sockaddr * addr, char *res) {
  void *res_addr;

  if (addr->sa_family == AF_INET) {
    struct sockaddr_in *ipv4 = (struct sockaddr_in *) addr;
    res_addr = &ipv4->sin_addr;
  } else {
    struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *) &addr;
    res_addr = &ipv6->sin6_addr;
  }
  inet_ntop(addr->sa_family, res_addr, res, INET6_ADDRSTRLEN);
}

void sigchld_handler() {
  // Automatically reap all exited child processes
  while (waitpid(-1, NULL, WNOHANG) > 0);
}

void sigterm_handler() {
  // Perform cleanup tasks (e.g., closing the socket)
  printf("\nReceived SIGTERM, shutting down server...\n");

  if (sockfd >= 0) {
    close(sockfd);
    printf("Server socket closed.\n");
  }

  fflush(NULL);
  exit(0); // Exit the program after cleanup
}

void signal_handler(int signum) {
  switch (signum) {
    case SIGCHLD:
      sigchld_handler();
      break;
    case SIGTERM:
      sigterm_handler();
      break;
    case SIGINT:
      sigterm_handler();
      break;
  }
}

// Create server socket  
// bind and listen on port
int create_server_socket() {
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  struct addrinfo *servinfo, *server; // will point to results
  int status;
  // get ip address and port numbers for host server
  if ((status = getaddrinfo(NULL, MY_PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    return 1;
  }

  if (servinfo == NULL) {
    printf("getaddrinfo: could not find result");
    return 1;
  }

  server = servinfo;
  // make a socket, bind it, listen on it
  sockfd = socket(server->ai_family, server->ai_socktype, server->ai_protocol);

  // allows us to reuse ip address and port 
  int yes;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
    printf("setsockopt: error: %d", errno);
    return 1;
  }

  if (bind(sockfd, server->ai_addr, server->ai_addrlen) == -1) {
    printf("bind: error");
    return 1;
  }

  if (listen(sockfd, QUEUE_SIZE) == -1) {
    printf("listen: error");
    return 1;
  }

  char ipstr[INET6_ADDRSTRLEN];
  addr_to_str(server->ai_addr, ipstr);
  printf("listening on %s:%s\n", ipstr, MY_PORT);
  fflush(NULL);
  freeaddrinfo(servinfo);
  return 0;
}
int init_signal_handler() {
  // Set up the signal handler using sigaction
  struct sigaction sa;
  sa.sa_handler = signal_handler;  // Set handler function
  sigemptyset(&sa.sa_mask);       // Don't block any other signals while handling SIGINT
  sa.sa_flags = 0;                // Default flags

  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("Error setting up SIGCHLD handler");
    return 1;
  }

  if (sigaction(SIGTERM, &sa, NULL) == -1) {
    perror("Error setting up SIGTERM handler");
    return 1;
  }
  return 0;
}

void print_client_ip(int new_socketfd) {
    struct sockaddr peer_addr = {0};
    socklen_t addr_len = sizeof(struct sockaddr);
    getpeername(new_socketfd, &peer_addr, &addr_len);

    char peer_name[INET6_ADDRSTRLEN];
    addr_to_str(&peer_addr, peer_name);
    printf("server: got connection from %s\n", peer_name);
    fflush(NULL);
}
int main() {
  if (create_server_socket() != 0) {
    return 1;
  }

  if (init_signal_handler() != 0) {
    return 1;
  }

  // accept incoming connections
  struct sockaddr_storage client_addr = {0};

  while(1) {
    socklen_t addr_size = sizeof(client_addr);
    int new_sockfd = accept(sockfd, (struct sockaddr *) &client_addr, &addr_size);

    if (new_sockfd == -1) {
      if (errno == EINTR) {
        // If accept() was interrupted by a signal, retry
        continue;
      }
      perror("accept");
      continue;
    }

    print_client_ip(new_sockfd);

    // handle connection in child process
    if (fork() == 0) { // in child
      close(sockfd); // child doesn't need listener
      if (handle_client(new_sockfd) != 0) {
        exit(1);
      }
      exit(0);
    }
    close(new_sockfd);
  }
  return 0;
}
