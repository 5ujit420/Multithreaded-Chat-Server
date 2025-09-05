#include "include/server.h"
#include <stdio.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: %s <port>\n", argv[0]);
    return 0;
  }

  run_server(init_server(argv[1]));

  return 0;
}
