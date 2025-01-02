#include "stdio.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include "string.h"

int main() {
  int status;
  char* site = "alexafazio.dev";
  struct addrinfo hints = {};
  struct addrinfo *servinfo, *winner; // will point to results
  char ipstr[INET6_ADDRSTRLEN];

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if ((status = getaddrinfo(site, "80", &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    return 1;
  }

  for (winner = servinfo; winner != NULL; winner = winner->ai_next) {
    void *addr;
    char *ipver;

    if (winner->ai_family == AF_INET) {
      ipver = "IPv4";
      struct sockaddr_in *ipv4 = (struct sockaddr_in *) winner->ai_addr;
      addr = &ipv4->sin_addr;
    } else {
      ipver = "IPv6";
      struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *) winner->ai_addr;
      addr = &ipv6->sin6_addr;
    }
    inet_ntop(winner->ai_family, addr, ipstr, sizeof ipstr);
    printf("%s: %s\n", ipver, ipstr);
  }
  freeaddrinfo(servinfo);
  return 0;
}
