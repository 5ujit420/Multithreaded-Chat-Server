#ifndef SERVER_H
#define SERVER_H

#include "client.h"

int init_server(char *PORT);

void run_server(int sock_fd);

#endif
