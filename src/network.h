#ifndef NETWORK_H
#define NETWORK_H

struct sockaddr;

void addr_to_str(struct sockaddr * addr, char *res);
void print_client_ip(int client_socket_fd);
int create_server_socket();

#endif // NETWORK_H
