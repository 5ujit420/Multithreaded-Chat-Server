#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <sys/types.h>

ssize_t send_all(int sock_fd, const void *buffer, size_t len);

ssize_t recv_all(int sock_fd, const void *buffer, size_t len);

#endif
