#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <time.h>

typedef enum {
  LOG_INFO,
  LOG_WARN,
  LOG_ERROR,
  LOG_DEBUG,
} log_level_t;

void log_message(log_level_t level, const char *fmt, ...);

#define MSG_SOCK_CREATED "Socket created"
#define MSG_SOCK_CREATE_FAIL "Socket creation failed"
#define MSG_SOCK_BOUND "Socket bound to port %s"
#define MSG_SOCK_CLOSED "Socket closed"
#define MSG_SERVER_LISTEN "Server started listening at port %s..."
#define MSG_SERVER_STOPING "Stoping server..."
#define MSG_SERVER_STOPED "Server stoped cleanly"
#define MSG_CLIENT_CONNECT "Client connected as %s"
#define MSG_CLIENT_DISCONNECT "Client %s disconnected"

#endif
