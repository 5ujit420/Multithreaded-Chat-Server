# Design Document: Multithreaded Chat Server

## Overview

This project implements a multithreaded TCP chat server in C.  
It allows multiple clients to connect concurrently, exchange messages, and use commands such as `/list` or `/quit`.  
The server demonstrates system-level programming concepts including socket programming, concurrency with POSIX threads, synchronization with mutexes, and modular software design.

---

## Objectives

- Support multiple clients simultaneously.
- Provide thread-safe message broadcasting to all connected users.
- Handle client lifecycle events (connect, disconnect, commands).
- Maintain logs of server activity with timestamps.
- Demonstrate clean modular code organization.

---

## Architecture

**Modules**

- `server.c` – Sets up server socket, binds, listens, and accepts client connections.
- `client_handler.c` – Manages threads, processes incoming messages, broadcasts to other clients.
- `logger.c` – Handles event logging (connections, disconnections, messages).
- `utils.c` – Helper functions for string operations, error handling, etc.
- `main.c` – Entry point; initializes server and coordinates modules.

**Headers (`include/`)**

- `server.h`, `client_handler.h`, `logger.h`, `utils.h` – Public interfaces for each module.

---

## Threading Model

- Each client connection is managed by a dedicated thread (`pthread_t`).
- A global client list maintains active clients.
- Access to the client list is synchronized with a `pthread_mutex_t`.
- Workflow per client thread:
  1. Receive messages from the client.
  2. If message is a command → handle accordingly.
  3. Otherwise, broadcast message to all connected clients.
  4. On disconnect, clean up resources and update global state.

---

## Data Structures

```c
// Represents a connected client
struct client {
    int socket_fd;
    char username[32];
};

```
