#include <sys/signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include "sig_utils.h"

void sigchld_handler() {
  // Automatically reap all exited child processes
  while (waitpid(-1, NULL, WNOHANG) > 0);
}

/*void sigterm_handler(int server_socket_fd) {*/
/*  // Perform cleanup tasks (e.g., closing the socket)*/
/*  printf("\nReceived SIGTERM, shutting down server...\n");*/
/**/
/*  if (server_socket_fd >= 0) {*/
/*    close(server_socket_fd);*/
/*    printf("Server socket closed.\n");*/
/*  }*/
/**/
/*  fflush(NULL);*/
/*  exit(0); // Exit the program after cleanup*/
/*}*/

void signal_handler(int signum) {
  switch (signum) {
    case SIGCHLD:
      sigchld_handler();
      break;
    case SIGTERM:
      /*sigterm_handler();*/
      break;
    case SIGINT:
      /*sigterm_handler();*/
      break;
  }
}
int init_signal_handler() {
  // Set up the signal handler using sigaction
  struct sigaction sa;
  sa.sa_handler = signal_handler;  // Set handler function
  sigemptyset(&sa.sa_mask);       // Don't block any other signals while handling SIGINT
  sa.sa_flags = 0;                // Default flags

  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("Error setting up SIGCHLD handler");
    return -1;
  }

  if (sigaction(SIGTERM, &sa, NULL) == -1) {
    perror("Error setting up SIGTERM handler");
    return -1;
  }
  return 0;
}
