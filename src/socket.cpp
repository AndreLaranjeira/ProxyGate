// Socket module - Source code.

// Includes:
#include "include/socket.h"

// Function implementations:
int close_socket(int fd) {

  // Shutdown the socket connections.
  // Argument options:
  //    0: Stop receiving data.
  //    1: Stop transmitting data and receiving ACKs.
  //    2: Stop receiving and transmitting data.
  shutdown(fd, 2);

  // Close the file descriptor:
  close(fd);

  return 0;

}

int connect_socket (int fd, struct sockaddr *addr, socklen_t len) {
  return connect(fd, addr, len);
}

ssize_t read_socket(int fd, char *buffer, size_t size) {

  ssize_t end;
  end = read(fd, buffer, size);
  buffer[end] = '\0';

  return (end >= 0) ? end : -1;

}
