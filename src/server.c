#include "../include/server.h"
#include "../include/logger.h"
#include <errno.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int init_server(char *PORT) {
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
    log_message(LOG_INFO, "Creating Socket.");

    sock_fd = socket(itr->ai_family, itr->ai_socktype, itr->ai_protocol);

    if (sock_fd == -1)
      continue;

    log_message(LOG_INFO, "Scoket created.");
    log_message(LOG_INFO, "Binding Socket");
    if (bind(sock_fd, itr->ai_addr, itr->ai_addrlen) == 0) {
      log_message(LOG_INFO, "Socket bound to port %s.", PORT);
      break;
    }

    close(sock_fd);
  }

  freeaddrinfo(sockList);

  if (itr == NULL) {
    log_message(LOG_ERROR, "Unable to create socket");
    exit(EXIT_FAILURE);
  }

  if (listen(sock_fd, 10) == -1) {
    log_message(LOG_ERROR, "listen: %s", strerror(errno));
    close(sock_fd);
    exit(EXIT_FAILURE);
  }

  log_message(LOG_INFO, "Server Listening at port %s...", PORT);

  return sock_fd;
}

void run_server(int sock_fd) {
  while (1) {
    add_client(sock_fd);
  }
}
