// Socket module - Source code.

// Includes:
#include "include/socket.h"

// Function implementations:
int connect_socket (int SOCKET, struct sockaddr *ADDR, socklen_t
                    LENGTH) {
  return connect(SOCKET, ADDR, LENGTH);
}
