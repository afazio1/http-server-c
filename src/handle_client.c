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
  char *uri;
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
   
  // parse tokens in first line by space
  // parse method
  char *first_token_ptr;
  char *first_token = strtok_r(line, " ", &first_token_ptr);
  size_t first_token_len = strlen(first_token);
  req->method = calloc(first_token_len + 1, sizeof(char));
  if (req->method == NULL) return 1;
  strncpy(req->method, first_token, first_token_len + 1);

  // parse uri
  char *second_token = strtok_r(NULL, " ", &first_token_ptr);
  size_t second_token_len = strlen(second_token);
  req->uri = calloc(second_token_len + 1, sizeof(char));
  if (req->uri == NULL) return 1;
  strncpy(req->uri, second_token, second_token_len + 1);

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
long get_fsize(FILE *fd) {
  if (fseek(fd, 0, SEEK_END) == -1) {
    fclose(fd);
    printf("error seeking file: %d\n", errno);
    return 1;
  }
  long fsize = ftell(fd);
  if (fsize == -1) {
    fclose(fd);
    printf("error telling file: %d\n", errno);
    return 1;
  }
  rewind(fd);
  return fsize;
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
    http_req_t req = {0};
    if (parse_http_request(buf, &req) == 1) {
      return 1;
    }
    // TODO: get rid of this
    printf("Method: %s\n", req.method);
    printf("URI: %s\n", req.uri);
    printf("Host: %s\n", req.host);
    printf("User-Agent: %s\n", req.user_agent);
    fflush(NULL);
    // get file based on uri
    char* filename = "";
    if (strncmp(req.uri, "/", 1) == 0) {
      filename = "index.html";
    }
    char temp[100];
    snprintf(temp, sizeof(temp), "../html%s%s", req.uri, filename);
    req.uri = realloc(req.uri, strlen(temp) + 1);
    strncpy(req.uri, temp, strlen(temp) + 1);
    printf("%s\n", req.uri);
    fflush(NULL);
    FILE *fd = fopen(req.uri, "r");
    if (fd == NULL) {
      printf("error opening file: %d\n", errno);
      return 1;
    }
    // get size of file
    long fsize = get_fsize(fd);

    // allocate memory for it
    char* page_buf = calloc(fsize + 1, sizeof(char));
    if (page_buf == NULL) {
      fclose(fd);
      return 1;
    }
    // read the file into buffer
    fread(page_buf, sizeof(char), fsize, fd);
    page_buf[fsize] = 0;

    // close file
    fclose(fd);
    // TODO: remove printfs
    printf("%s\n", page_buf);
    fflush(NULL);

    // TODO: send a response back
    /*int bytes_sent = send(client_sock, buf, bytes_recv, 0);*/
    /*if (bytes_sent == -1) {*/
    /*  return 1;*/
    /*}*/

    if (req.uri != 0) free(req.uri);
    if (req.method != 0) free(req.method);
    if (req.host != 0) free(req.host);
    if (req.user_agent != 0) free(req.user_agent);

  }
   
  close(client_sock);
  return 0;
} 
