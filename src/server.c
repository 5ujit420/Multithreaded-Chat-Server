#include "../include/server.h"
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

int server_init() {
  struct addrinfo hints, *sockList;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  getaddrinfo(NULL, PORT, &hints, &sockList);

  return 0;
}
