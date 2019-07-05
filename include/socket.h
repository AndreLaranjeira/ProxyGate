// Socket module - Header file.

/**
 * @file socket.h
 * @brief Socket module - Header file.
 *
 * The socket module contains function wrappers and small abstractions of
 * socket related functionalities. This header file contains a header guard,
 * library includes and the function headers for this module.
 *
 */

// Header guard:
#ifndef SOCKET_H
#define SOCKET_H

// Library includes:
#include <sys/socket.h>
#include <unistd.h>

// Function headers:
int close_socket(int);
int connect_socket (int, struct sockaddr*, socklen_t);
ssize_t read_socket(int, char*, size_t);

#endif // SOCKET_H
