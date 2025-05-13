#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <string.h>
#include "handle_client.h"

#define BUFFER_SIZE 4096
#define CRLF "\r\n"
#define INT32_DIGITS 32
#define INDEX_FILENAME "index.html"

void cleanup(char **buf) {
  // free(NULL) no operation is performed
  free(*buf);
  *buf = NULL;
}

typedef struct {
  char *method;
  char *uri;
  char *host;
  char *user_agent;
} http_req_t;

typedef struct {
  char *version;
  char *status;
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
  if (req->method == NULL) return -1;
  strncpy(req->method, first_token, first_token_len + 1);

  // parse uri
  char *second_token = strtok_r(NULL, " ", &first_token_ptr);
  size_t second_token_len = strlen(second_token);
  req->uri = calloc(second_token_len + 1, sizeof(char));
  if (req->uri == NULL) {
    goto free_stuff;
  }
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
          if (req->host == NULL) {
            goto free_stuff;
          }
          strncpy(req->host, token, token_len + 1);
        } else if (field == req->user_agent) {
          req->user_agent = calloc(token_len + 1, sizeof(char));
          if (req->user_agent == NULL) {
            goto free_stuff;
          }
          strncpy(req->user_agent, token, token_len + 1);
        }
      }
      token = strtok_r(NULL, " ", &token_ptr);
    }
  }

  return 0;

free_stuff:
  cleanup(&req->method);
  cleanup(&req->uri);
  cleanup(&req->host);
  cleanup(&req->user_agent);
  return -1;
}

char * create_http_response(char **content, http_res_t *res) {
  res->status_line.status = "200";
  res->status_line.reason = "OK";
  res->status_line.version = "HTTP/1.1";

  if (content == NULL) {
    res->status_line.status = "500";
    res->status_line.reason = "Server Error";
  }

  res->content_length = strlen(*content);
  res->content_type = "text/html";
  
  // create the res string
  char* res_str = calloc(res->content_length + 100, sizeof(char));
  strcat(res_str, res->status_line.version);
  strcat(res_str, " ");
  strcat(res_str, res->status_line.status);
  strcat(res_str, " ");
  strcat(res_str, res->status_line.reason);
  strcat(res_str, CRLF);
  strcat(res_str, "Content-Type: ");
  strcat(res_str, res->content_type);
  strcat(res_str, CRLF);
  strcat(res_str, "Content-Length: ");
  char* content_len_str = calloc(INT32_DIGITS + 1, sizeof(char)); // 32 digits + 1 for null terminator
  if (content_len_str == NULL) {
    return NULL;
  }
  sprintf(content_len_str, "%d", res->content_length);
  strcat(res_str, content_len_str);
  cleanup(&content_len_str);
  strcat(res_str, CRLF);
  strcat(res_str, CRLF);
  strcat(res_str, *content);
  return res_str;
}

long get_fsize(FILE *fd) {
  if (fseek(fd, 0, SEEK_END) == -1) {
    fclose(fd);
    printf("error seeking file: %d\n", errno);
    return -1;
  }
  long fsize = ftell(fd);
  if (fsize == -1) {
    fclose(fd);
    printf("error telling file: %d\n", errno);
    return -1;
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
      return -1;
    }
    buf[bytes_recv] = 0;
    http_req_t req = {0};
    if (parse_http_request(buf, &req) == 1) {
      return -1;
    }
    // get file based on uri
    char* filename = "";
    if (strncmp(req.uri, "/", 1) == 0) {
      filename = INDEX_FILENAME;
    }
    char temp[100];
    snprintf(temp, sizeof(temp), "../html%s%s", req.uri, filename);
    req.uri = realloc(req.uri, strlen(temp) + 1);
    strncpy(req.uri, temp, strlen(temp) + 1);

    FILE *fd = fopen(req.uri, "r");
    if (fd == NULL) {
      printf("error opening file: %d\n", errno);
      cleanup(&req.method);
      cleanup(&req.uri);
      cleanup(&req.host);
      cleanup(&req.user_agent);
      return -1;
    }
    // get size of file
    long fsize = get_fsize(fd);

    // allocate memory for it
    char* page_buf = calloc(fsize + 1, sizeof(char));
    if (page_buf == NULL) {
      fclose(fd);
      cleanup(&req.method);
      cleanup(&req.uri);
      cleanup(&req.host);
      cleanup(&req.user_agent);
      return -1;
    }
    // read the file into buffer
    fread(page_buf, sizeof(char), fsize, fd);
    if (ferror(fd) != 0) {
      fclose(fd);
      cleanup(&req.method);
      cleanup(&req.uri);
      cleanup(&req.host);
      cleanup(&req.user_agent);
      cleanup(&page_buf);
      return -1;
    }
    page_buf[fsize] = 0;

    // close file
    fclose(fd);

    // send a response back
    http_res_t res = {0};
    char* res_str = create_http_response(&page_buf, &res);
    if (res_str == NULL) {
      printf("error creating response: %d\n", errno);
      cleanup(&req.method);
      cleanup(&req.uri);
      cleanup(&req.host);
      cleanup(&req.user_agent);
      cleanup(&page_buf);
      return -1;
    }
    printf("%s\n", res_str);
    fflush(NULL);
    int bytes_sent = send(client_sock, res_str, strlen(res_str), 0);
    if (bytes_sent == -1) {
      cleanup(&req.method);
      cleanup(&req.uri);
      cleanup(&req.host);
      cleanup(&req.user_agent);
      cleanup(&page_buf);
      cleanup(&res_str);
      return -1;
    }
    
    cleanup(&req.method);
    cleanup(&req.uri);
    cleanup(&req.host);
    cleanup(&req.user_agent);
    cleanup(&page_buf);
    cleanup(&res_str);
  }
  printf("Closed client socket\n");
  close(client_sock);
  return 0;
} 
