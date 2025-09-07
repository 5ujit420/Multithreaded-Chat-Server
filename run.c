#include "include/server.h"
#include <stdio.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: %s <port>\n", argv[0]);
    return 0;
  }

  char *port = argv[1];
  int server_fd = start_server(port);
  run_server(server_fd);
  stop_server(server_fd);

  return 0;
}
