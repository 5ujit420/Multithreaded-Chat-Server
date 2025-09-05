#include "../include/utils.h"
#include "../include/logger.h"
#include <errno.h>
#include <string.h>
#include <sys/socket.h>

ssize_t send_all(int sock_fd, const void *buffer, size_t len) {
  size_t total_sent = 0;
  const char *ptr = buffer;

  while (total_sent < len) {
    ssize_t sent = send(sock_fd, ptr + total_sent, len - total_sent, 0);

    if (sent <= 0) {
      log_message(LOG_ERROR, "send: %s", strerror(errno));
      return -1;
    }

    total_sent += sent;
  }

  return total_sent;
}

ssize_t recv_all(int sock_fd, const void *buffer, size_t len) {
  size_t total_recv = 0;
  const char *ptr = buffer;

  while (total_recv < len) {
    ssize_t recvd = send(sock_fd, ptr + total_recv, len - total_recv, 0);

    if (recvd <= 0) {
      log_message(LOG_ERROR, "recv: %s", strerror(errno));
      return -1;
    }

    total_recv += recvd;
  }

  return total_recv;
}
