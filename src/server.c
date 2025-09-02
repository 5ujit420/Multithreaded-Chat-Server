// #define _POSIX_C_SOURCE 200809L

#include "../include/server.h"
#include "../include/logger.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static client_t *clients[MAX_CLIENTS];
static pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;
static int client_count = 0;

static volatile sig_atomic_t keep_running = 1;

// =======================
// Helper function
// =======================
void handle_sigint(int sig) {
  (void)sig;
  keep_running = 0;
  log_message(LOG_INFO, "Shutting down server...");
}

static int add_client(client_t *client) {
  pthread_mutex_lock(&client_mutex);

  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (!clients[i]) {
      clients[i] = client;
      client_count++;
      pthread_mutex_unlock(&client_mutex);
      return 0;
    }
  }

  pthread_mutex_unlock(&client_mutex);
  log_message(LOG_ERROR, "Max client (%d) slot full", MAX_CLIENTS);

  return -1;
}

static void remove_client(int sockfd) {
  pthread_mutex_lock(&client_mutex);

  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (clients[i] && clients[i]->sockfd == sockfd) {
      close(clients[i]->sockfd);
      free(clients[i]);
      clients[i] = NULL;
      client_count--;
      pthread_mutex_unlock(&client_mutex);
      return;
    }
  }

  pthread_mutex_unlock(&client_mutex);

  log_message(LOG_ERROR, "Client with sockfd %d not found", sockfd);
}

static void *handle_client(void *arg) {
  client_t *client = (client_t *)arg;
  char buffer[BUFFER_SIZE];

  const char *ask_username = "Enter username: ";
  if (send(client->sockfd, ask_username, strlen(ask_username), 0) < 0) {
    log_message(LOG_ERROR, "Falied to send ask username prompt: %s",
                strerror(errno));
    remove_client(client->sockfd);
    pthread_exit(NULL);
  }

  memset(buffer, 0, sizeof(buffer));
  ssize_t bytes = recv(client->sockfd, buffer, sizeof(buffer), 0);

  if (bytes <= 0) {
    log_message(LOG_ERROR, "Failed to recv username: %s", strerror(errno));
    remove_client(client->sockfd);
    pthread_exit(NULL);
  }

  buffer[strcspn(buffer, "\n")] = '\0';
  printf("test: %s", buffer);
  size_t i = 0;
  while (i < sizeof(buffer)) {
    printf("%c", buffer[i]);
    if (buffer[i] == '\n')
      printf("yes\n");
    i++;
  }

  strncpy(client->username, buffer, sizeof(client->username) - 1);
  client->username[sizeof(client->username) - 1] = '\0';

  char welcome_msg[BUFFER_SIZE];
  snprintf(welcome_msg, sizeof(welcome_msg), "!!!%s joined the chat!!!",
           client->username);
  server_broadcast(welcome_msg, client->sockfd);

  while (1) {
    memset(buffer, 0, sizeof(buffer));
    ssize_t bytes = recv(client->sockfd, buffer, sizeof(buffer), 0);

    if (bytes < 0) {
      if (errno == EINTR)
        continue;
      if (errno == EAGAIN || errno == EWOULDBLOCK)
        continue;

      log_message(LOG_ERROR, "%s disconnected with error: %s", client->username,
                  strerror(errno));
      remove_client(client->sockfd);
      pthread_exit(NULL);
    }

    if (bytes == 0) {
      log_message(LOG_INFO, "%s disconnected", client->username);
      remove_client(client->sockfd);
      pthread_exit(NULL);
    }

    buffer[strcspn(buffer, "\n")] = '\0';

    log_message(LOG_INFO, "Message from %s: %s", client->username, buffer);

    server_broadcast(buffer, client->sockfd);
  }

  return NULL;
}

// ========================
// API Implementation
// ========================
int server_init(int port) {

  struct addrinfo init, *socketList;

  memset(&init, 0, sizeof(init));
  init.ai_family = AF_UNSPEC;
  init.ai_socktype = SOCK_STREAM;
  init.ai_flags = AI_PASSIVE;

  char port_str[10];
  snprintf(port_str, sizeof(port_str), "%d", port);

  int rv;
  if ((rv = getaddrinfo(NULL, port_str, &init, &socketList)) != 0) {
    log_message(LOG_ERROR, "getaddrinfo: %s", gai_strerror(rv));
    return -1;
  }

  struct addrinfo *itr;
  int listener = -1;
  int yes = 1;
  for (itr = socketList; itr != NULL; itr = itr->ai_next) {
    listener = socket(itr->ai_family, itr->ai_socktype, itr->ai_protocol);

    if (listener < 0)
      continue;

    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
      close(listener);
      log_message(LOG_ERROR, "setsockopt failed: %s", strerror(errno));
      log_message(LOG_INFO, "continuing with other socket...");
      continue;
    }

    if (bind(listener, itr->ai_addr, itr->ai_addrlen) != 0) {
      close(listener);
      continue;
    }

    break;
  }

  if (itr == NULL) {
    log_message(LOG_ERROR, "socket creation failed: %s", strerror(errno));
    return -1;
  }

  freeaddrinfo(socketList);

  if (listen(listener, 10) < 0) {
    log_message(LOG_ERROR, "listen failed: %s", strerror(errno));
    return -1;
  }

  return listener;
}

int server_accept(int server_fd, client_t *client) {
  socklen_t addr_len = sizeof(client->addr);
  for (;;) {
    int new_sock =
        accept(server_fd, (struct sockaddr *)&client->addr, &addr_len);
    if (new_sock >= 0) {
      client->sockfd = new_sock;
      return new_sock;
    }

    if (errno == EINTR) {
      if (!keep_running)
        return -1;
      continue; // interrupted by signal, try again
    }

    log_message(LOG_ERROR, "client accept failed: %s", strerror(errno));
    return -1;
  }
}

void server_run(int server_fd) {
  struct sigaction sa;
  sa.sa_handler = handle_sigint;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  if (sigaction(SIGINT, &sa, NULL) == -1) {
    perror("sigaction");
    exit(EXIT_FAILURE);
  }

  while (keep_running) {
    client_t *client = calloc(1, sizeof(*client));

    if (!client) {
      log_message(LOG_ERROR, "malloc failed");
      sleep(1);
      continue;
    }

    int client_fd = server_accept(server_fd, client);
    if (client_fd < 0) {
      free(client);
      if (!keep_running)
        break;
      continue;
    }

    if (add_client(client) < 0) {
      log_message(LOG_ERROR, "failed to add client");
      close(client->sockfd);
      free(client);
      continue;
    }

    if (pthread_create(&client->thread_id, NULL, handle_client,
                       (void *)client) != 0) {
      log_message(LOG_ERROR, "failed to create thread");
      remove_client(client->sockfd);
      continue;
    }
    log_message(LOG_INFO, "New CLIENT connected: %s", client->username);
    pthread_detach(client->thread_id);
  }
}

void server_broadcast(const char *msg, int sockfd) {
  int fds[MAX_CLIENTS];
  int n = 0;

  pthread_mutex_lock(&client_mutex);
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (clients[i] && clients[i]->sockfd != sockfd) {
      fds[n++] = clients[i]->sockfd;
    }
  }
  pthread_mutex_unlock(&client_mutex);

  for (int i = 0; i < n; i++) {
    if (send(fds[i], msg, strlen(msg), 0) < 0) {
      log_message(LOG_ERROR, "send failed: %s", strerror(errno));
    }
  }
}

void server_remove_client(int client_fd) { remove_client(client_fd); }

void server_shutdown(int server_fd) {
  close(server_fd);
  pthread_mutex_destroy(&client_mutex);
}
