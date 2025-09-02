#ifndef SERVER_H
#define SERVER_H

#include <netdb.h>
#include <pthread.h>
#include <stdlib.h>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024

typedef struct {
  int index;
  int sockfd;                   // client socket file discriptor
  struct sockaddr_storage addr; // client IPv4 or IPv6 address
  pthread_t thread_id;          // process id for client handling
  char username[32];            // client username
} client_t;

// Initialising server socket, bind and listen
// Returns listener socket file discriptor on success, -1 on failure
int server_init(int port);

void server_run(int server_fd);

int server_accept(int server_fd, client_t *client);

void server_broadcast(const char *buffer, int sockfd);

void server_remove_client(int client_fd);

void server_shutdown(int server_fd);

#endif
