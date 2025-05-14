#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include "sig_utils.h"
#include "network.h"
#include "handle_client.h"


int server_socket_fd;

int main() {
  if ((server_socket_fd = create_server_socket()) == -1) {
    return 1;
  }

  if (init_signal_handler() != 0) {
    return 1;
  }

  // accept incoming connections
  struct sockaddr_storage client_addr = {0};

  while(1) {
    socklen_t addr_size = sizeof(client_addr);
    int new_client_socket_fd = accept(server_socket_fd, (struct sockaddr *) &client_addr, &addr_size);

    if (new_client_socket_fd == -1) {
      if (errno == EINTR) {
        // If accept() was interrupted by a signal, retry
        continue;
      }
      perror("accept");
      continue;
    }

    print_client_ip(new_client_socket_fd);

    // handle connection in child process
    if (fork() == 0) { // in child
      close(server_socket_fd); // child doesn't need listener
      if (handle_client(new_client_socket_fd) == -1) {
        exit(1);
      }
      exit(0);
    }
    close(new_client_socket_fd);
  }
  return 0;
}
