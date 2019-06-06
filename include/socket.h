// Socket module - Header file.

// Header guard:
#ifndef SOCKET_H
#define SOCKET_H

// Library includes:
#include <sys/socket.h>

// Function headers:
int connect_socket (int, struct sockaddr*, socklen_t);

#endif // SOCKET_H
