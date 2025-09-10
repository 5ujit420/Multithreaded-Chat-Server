#include "../include/server.h"
#include "../include/logger.h"
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

volatile sig_atomic_t keep_running;

static void handler(int sig) {
  keep_running = 0;
  return;
}

int start_server(char *PORT) {
  struct addrinfo hints, *sockList;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if (getaddrinfo(NULL, PORT, &hints, &sockList) < 0) {
    log_message(LOG_ERROR, "getaddrinfo: %s", gai_strerror(errno));
    exit(1);
  }

  struct addrinfo *itr;
  int sock_fd;
  for (itr = sockList; itr != NULL; itr = itr->ai_next) {

    sock_fd = socket(itr->ai_family, itr->ai_socktype, itr->ai_protocol);

    if (sock_fd == -1)
      continue;

    log_message(LOG_INFO, MSG_SOCK_CREATED);
    if (bind(sock_fd, itr->ai_addr, itr->ai_addrlen) == 0) {
      log_message(LOG_INFO, MSG_SOCK_BOUND, PORT);
      break;
    }

    close(sock_fd);
  }

  freeaddrinfo(sockList);

  if (itr == NULL) {
    log_message(LOG_ERROR, MSG_SOCK_CREATE_FAIL);
    exit(EXIT_FAILURE);
  }

  if (listen(sock_fd, 10) == -1) {
    log_message(LOG_ERROR, "listen: %s", strerror(errno));
    close(sock_fd);
    exit(EXIT_FAILURE);
  }

  log_message(LOG_INFO, MSG_SERVER_LISTEN, PORT);

  return sock_fd;
}

void run_server(int sock_fd) {
  struct sigaction sa;
  sa.sa_handler = handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  if (sigaction(SIGINT, &sa, NULL) == -1) {
    log_message(LOG_ERROR, "sigaction: %s", strerror(errno));
    exit(1);
  }

  keep_running = 1;
  while (keep_running) {
    add_client(sock_fd);
  }
}

void stop_server(int sock_fd) {
  printf("\n");
  log_message(LOG_INFO, MSG_SERVER_STOPING);
  if (remove_all_clients() < 0) {
    log_message(LOG_ERROR, "remove_all_clients: failed");
    exit(1);
  }

  close(sock_fd);
  log_message(LOG_INFO, MSG_SOCK_CLOSED);
  log_message(LOG_INFO, MSG_SERVER_STOPED);
}
