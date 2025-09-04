#include "../include/client.h"
#include "../include/logger.h"
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

client_t *clients[MAX_CLIENTS];

// ======================
//      Constructor
// ======================
client_t *constructor(int sock_fd, struct sockaddr_storage addr,
                      socklen_t addr_len, char *username) {

  client_t *client = (client_t *)calloc(1, sizeof(client_t));

  client->sock_fd = sock_fd;
  client->addr = addr;
  client->addrlen = addr_len;
  client->username = username;

  return client;
}

// =====================
//        Helper
// =====================
static char *get_username(int sock_fd) {
  const char *prompt = "Enter Username (max 20 characters): ";
  char *username = (char *)calloc(21, 1);
  char buffer[64];

  if (!username) {
    log_message(LOG_ERROR, "calloc failed: %s", strerror(errno));
    return NULL;
  }

  while (1) {
    if (send(sock_fd, prompt, strlen(prompt), 0) == -1) {
      log_message(LOG_ERROR, "send failed: %s", strerror(errno));
      continue;
    }

    int bytes = recv(sock_fd, username, 20, 0);
    if (bytes <= 0) {
      log_message(LOG_ERROR, "Client disconnected");
      free(username);
      return NULL;
    }

    username[bytes] = '\0';
    username[strcspn(username, "\r\n")] = '\0';

    if (strlen(username) == 0)
      continue;

    break;
  }

  return username;
}

static void broadcast(char *msg, int sock_fd) {
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (clients[i] && clients[i]->sock_fd != sock_fd) {
      if (send(clients[i]->sock_fd, msg, strlen(msg), 0) < 0) {
        log_message(LOG_ERROR, "Failed to send message to %s",
                    clients[i]->username);
        continue;
      }
    }
  }
}

static void *comm_handler(void *arg) {
  pthread_detach(pthread_self());

  client_t *client = (client_t *)arg;
  int sock_fd = client->sock_fd;
  char *username = client->username;

  log_message(LOG_INFO, "Client %s connected", username);

  char buffer[1024];
  char msg[2048];
  int bytes;

  while (1) {
    memset(buffer, 0, sizeof(buffer));
    bytes = recv(sock_fd, buffer, sizeof(buffer) - 1, 0);

    if (bytes == 0) {
      log_message(LOG_INFO, "Client %s disconnected.", username);
      remove_client(sock_fd);
      return NULL;
    }

    if (bytes < 0) {
      log_message(LOG_ERROR, "Client %s disconnected with error: %s", username,
                  strerror(errno));
      remove_client(sock_fd);
      return NULL;
    }

    buffer[bytes] = '\0';

    snprintf(msg, sizeof(msg), "%s: %s", username, buffer);

    broadcast(msg, sock_fd);
  }

  return NULL;
}

// =======================
//         API
// =======================
//
// Add new client to clients array
// accepts client -> create new thread -> client_handler
// return 0 on success and <0 on failure
int add_client(int server_fd) {

  int i;
  for (i = 0; i < MAX_CLIENTS; i++) {
    if (!clients[i]) {
      break;
    }
  }

  if (i == MAX_CLIENTS) {
    return -1;
  }

  struct sockaddr_storage addr;
  socklen_t addr_len = sizeof(addr);
  int sock_fd = accept(server_fd, (struct sockaddr *)&addr, &addr_len);

  if (sock_fd < 0) {
    log_message(LOG_ERROR, "accept: %s", strerror(errno));
    close(sock_fd);
    return -2;
  }

  char *username = get_username(sock_fd);

  if (username == NULL) {
    return -3;
  }

  clients[i] = constructor(sock_fd, addr, addr_len, username);

  pthread_t pid;
  if (pthread_create(&pid, NULL, comm_handler, (void *)clients[i]) != 0) {
    log_message(LOG_ERROR, "pthread: %s", strerror(errno));
    return -4;
  }

  clients[i]->pid = pid;

  return 0;
}

// Removes existing client from clients array
// finds client -> closes fd -> detach thread -> free memo
// return 0 on success and <0 on failure
int remove_client(int sock_fd) {

  int i;
  for (i = 0; i < MAX_CLIENTS; i++) {
    if (clients[i] && clients[i]->sock_fd == sock_fd) {
      close(clients[i]->sock_fd);
      free(clients[i]->username);
      free(clients[i]);
      clients[i] = NULL;
      return 0;
    }
  }

  return -1;
}
