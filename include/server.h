#ifndef SERVER_H
#define SERVER_H

#include "client.h"
#include <sys/socket.h>

int start_server(char *PORT);

void run_server(int sock_fd);

void stop_server(int sock_fd);

#endif
