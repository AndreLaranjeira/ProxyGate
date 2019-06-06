// Socket module - Header file.

// Header guard:
#ifndef SOCKET_H
#define SOCKET_H

// Library includes:
#include <sys/socket.h>
#include <unistd.h>

// Function headers:
int close_socket(int);
int connect_socket (int, struct sockaddr*, socklen_t);
int read_socket(int, void*, size_t);

#endif // SOCKET_H
