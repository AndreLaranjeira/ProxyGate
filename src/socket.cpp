// Socket module - Source code.

/**
 * @file socket.cpp
 * @brief Socket module - Source code.
 *
 * The socket module contains function wrappers and small abstractions of
 * socket related functionalities. This source file contains the function
 * implementations for this module.
 *
 */

// Includes:
#include "include/socket.h"

// Function implementations:

/**
 * @fn int close_socket(int fd)
 * @brief Function to close the connections of a socket.
 * @param fd File descriptor for the socket whose connections will be closed.
 * @return Returns 0 when successfully executed.
 *
 * Closes the connections of a socket by calling the shutdown function from
 * 'sys/socket.h' with the socket file descriptor and a HOW argument of 0.
 * This causes the socket to only stop receiving data. We them close the file
 * descriptor of the socket with the close function.
 *
 */

int close_socket(int fd) {

  // Shutdown the socket connections.
  // Argument options:
  //    0: Stop receiving data.
  //    1: Stop transmitting data and receiving ACKs.
  //    2: Stop receiving and transmitting data.
  shutdown(fd, 0);

  // Close the file descriptor:
  close(fd);

  return 0;

}

/**
 * @fn int connect_socket (int fd, struct sockaddr *addr, socklen_t len)
 * @brief Wrapper function for the connect 'sys/socket.h' function.
 * @param fd File descriptor for the socket who will start the connection.
 * @param addr Address of the socket who receives the connection.
 * @param len Length of the socket who receives the connection.
 * @return Returns 0 when the successfully executed and -1 if an error occurs.
 *
 * A simple wrapper function for the connect function in 'sys/socket.h'. This
 * function doesn't implement anything new, but is necessary because a function
 * named 'connect' already exists in the 'QObject' module, overwriting the
 * connect function definition from 'sys/socket.h'.
 *
 */

int connect_socket (int fd, struct sockaddr *addr, socklen_t len) {
  return connect(fd, addr, len);
}

/**
 * @fn ssize_t read_socket(int fd, char *buffer, size_t size)
 * @brief Wrapper function for the read function in 'unistd.h'.
 * @param fd File descriptor for the socket where we read data from.
 * @param buffer Location to store the data read.
 * @param size Number of bytes to be read from the socket.
 * @return Returns the number of bytes read on success and -1 if an error
 * occurs.
 *
 * A simple wrapper function for the read function in 'unistd.h'. This
 * function automatically adds a '\0' to the end of the information read.
 *
 */

ssize_t read_socket(int fd, char *buffer, size_t size) {

  ssize_t end;
  end = read(fd, buffer, size);
  buffer[end] = '\0';

  return (end >= 0) ? end : -1;

}
