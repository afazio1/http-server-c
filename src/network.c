#include "network.h"
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>

#define QUEUE_SIZE 20
#define MY_PORT "6969"

/**
* addr_to_str - Convert a socket address to a human-readable string
* @addr: Pointer to a sockeraddr struct, which may be IPv4 or IPv6
* @res: Pointer to output buffer to store the resulting IP address
*
* This function detects whether the provided socket address is IPv4 or IPv6,
* extracts the corresponding IP address, and converts it to a human-readable
* string using inet_ntop(). The result is stored in the buffer pointed to by res.
*
* Note:
* - The res buffer must be large enough to hold an IPv6 string (at least INET6_ADDRSTRLEN).
* - The function does not return a value; the result is written directly to res.
*/
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
/**
* print_client_ip - Prints the connected client's IP to STDOUT
* @client_socket_fd: File descriptor for the client's socket
*/
void print_client_ip(int client_socket_fd) {
  struct sockaddr peer_addr = {0};
  socklen_t addr_len = sizeof(struct sockaddr);
  getpeername(client_socket_fd, &peer_addr, &addr_len);

  char peer_name[INET6_ADDRSTRLEN];
  addr_to_str(&peer_addr, peer_name);
  printf("server: got connection from %s\n", peer_name);
  fflush(NULL);
}

/**
* create_server_socket - Resolves local IP address, creates a server socket, binds them, listens on port
*
* Return: a file descriptor representing the server socket
*/
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
    return -1;
  }

  if (servinfo == NULL) {
    printf("getaddrinfo: could not find result");
    return -1;
  }

  server = servinfo;
  // make a socket, bind it, listen on it
  int sockfd = socket(server->ai_family, server->ai_socktype, server->ai_protocol);

  // allows us to reuse ip address and port 
  int yes;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
    printf("setsockopt: error: %d", errno);
    return -1;
  }

  if (bind(sockfd, server->ai_addr, server->ai_addrlen) == -1) {
    printf("bind: error");
    return -1;
  }

  if (listen(sockfd, QUEUE_SIZE) == -1) {
    printf("listen: error");
    return -1;
  }

  char ipstr[INET6_ADDRSTRLEN];
  addr_to_str(server->ai_addr, ipstr);
  printf("listening on %s:%s\n", ipstr, MY_PORT);
  fflush(NULL);
  freeaddrinfo(servinfo);
  return sockfd;
}

