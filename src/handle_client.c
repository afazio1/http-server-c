#include "stdio.h"
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include "string.h"
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <string.h>
#include "handle_client.h"

#define BUFFER_SIZE 4096
#define CRLF "\r\n"

typedef struct {
  char *method;
  char *host;
  char *user_agent;
} http_req_t;

typedef struct {
  char *version;
  uint32_t status;
  char *reason;
} state_line_t;

typedef struct {
  state_line_t status_line;
  char *content_type;
  uint32_t content_length;
} http_res_t;


int parse_http_request(char * msg, http_req_t *req) {
  // parse each line using CRLF
  char *saveptr;
  char *line = strtok_r(msg, CRLF, &saveptr);
   
  // parse first line by space
  char *first_token_ptr;
  char *token = strtok_r(line, " ", &first_token_ptr);
  size_t token_len = strlen(token);
  req->method = calloc(token_len + 1, sizeof(char));
  if (req->method == NULL) return 1;
  strncpy(req->method, token, token_len + 1);

  char *field;
  while (line != NULL) {
    // get line
    line = strtok_r(NULL, CRLF, &saveptr);

    // get tokens in line
    char *token_ptr;
    char *token = strtok_r(line, " ", &token_ptr);
    
    while (token != NULL) {
      // this is a key
      size_t token_len = strlen(token);
      if (strncmp("Host:", token, token_len) == 0) { 
        field = req->host;
      } else if (strncmp("User-Agent:", token, token_len) == 0) {
        field = req->user_agent;
      } else { // this is the value
        if (field == req->host) {
          req->host = calloc(token_len + 1, sizeof(char));
          if (req->host == NULL) return 1;
          strncpy(req->host, token, token_len + 1);
        } else if (field == req->user_agent) {
          req->user_agent = calloc(token_len + 1, sizeof(char));
          if (req->user_agent == NULL) return 1;
          strncpy(req->user_agent, token, token_len + 1);
        }
      }
      token = strtok_r(NULL, " ", &token_ptr);
    }
  }
  // parse subsequent lines to get key-value pair
  return 0;
}

void create_http_request() {

}
// HTTP/1.1
int handle_client(int client_sock) {
  // recv request
  char buf[BUFFER_SIZE];
  int bytes_recv = 0;
  while ((bytes_recv = recv(client_sock, buf, sizeof(buf), 0)) != 0) { // gets msg up to BUFFER_SIZE bytes
    if (bytes_recv == -1) {
      return 1;
    }
    buf[bytes_recv] = 0;
    // TODO: parse request
    http_req_t req = {0};
    if (parse_http_request(buf, &req) == 1) {
      return 1;
    }
    // TODO: get rid of this
    printf("Method: %s\n", req.method);
    printf("Host: %s\n", req.host);
    printf("User-Agent: %s\n", req.user_agent);
    fflush(NULL);
    /*int bytes_sent = send(client_sock, buf, bytes_recv, 0);*/
    /*if (bytes_sent == -1) {*/
    /*  return 1;*/
    /*}*/
    if (req.method != 0) free(req.method);
    if (req.host != 0) free(req.host);
    if (req.user_agent != 0) free(req.user_agent);

  }
  // TODO: read data
  // TODO: do something
  // TODO: send a response back

  close(client_sock);
  return 0;
} 
