#include "../include/logger.h"
#include "../include/server.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int port = atoi(argv[1]);

  int server_fd = server_init(port);
  log_message(LOG_INFO, "Server started. Waiting for clients...");

  server_run(server_fd);

  server_shutdown(server_fd);
  log_message(LOG_INFO, "Server terminated.");
  return 0;
}
