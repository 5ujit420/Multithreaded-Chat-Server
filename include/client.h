#ifndef CLIENT_H
#define CLIENT_H

#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>

#define MAX_CLIENTS 100

typedef struct Client {
  int sock_fd;
  struct sockaddr_storage addr;
  socklen_t addrlen;
  char *username;
  pthread_t pid;
} client_t;

client_t *constructor(int sock_fd, struct sockaddr_storage addr,
                      socklen_t addrlen, char *username);

int add_client(int server_fd);

int remove_client(int sock_fd);

#endif
