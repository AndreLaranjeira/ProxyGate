// Server module - Source code.

// Includes:
#include "include/server.h"

// Class methods:
Server::Server(in_port_t port_number) : logger("Server") {

  // Configure the socket incoming address:
  address.sin_family = AF_INET;           // Internet address format.
  address.sin_addr.s_addr = INADDR_ANY;   // Accept connections from all IPs.
  address.sin_port = htons(port_number);  // Port number.

}

Server::~Server() {

}

bool Server::isRunning() {
  return running;
}

int Server::init() {

  // Variable declaration:
  int opt = 1;

  // Creating the socket:
  if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    logger.error("Failed to create server socket!");
    return -1;
  }

  // Configure socket to reuse addresses and ports:
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt)) != 0) {
    logger.error("Failed to configure server socket options!");
    return -1;
  }

  // Bind the socket to the selected port:
  if (bind(server_fd, reinterpret_cast<struct sockaddr*> (&address),
           sizeof(address)) != 0) {
    logger.error("Failed to bind server to the selected port!");
    return -1;
  }

  // And there we go! This should make the server ready to begin listening for
  // requests. Just call Server::run() to begin.
  logger.success("Server initialized!");
  return 0;

}

int Server::run() {

  int addrlen = sizeof(address);

  // Start listening for requests:
  if(listen(server_fd, SERVER_BACKLOG) < 0) {
    logger.error("Failed configure socket to accept connections!");
    return -1;
  }

  running = true;   // This will be useful later.

  while(running) {

    // Accept an incoming client connection (blocking function call):
    if((client_fd = accept(server_fd,
                            reinterpret_cast<struct sockaddr*> (&address),
                            reinterpret_cast<socklen_t*> (&addrlen))) == 0) {
      read(client_fd, request, 8192);
      logger.info("Received some message!");
    }

    else if(!running)
      break;

    else {
      logger.error("Failed to accept an incoming connection!");
      return -1;
    }

  }

  logger.success("Server successfully shutdown!");
  return 0;

}

int Server::stop() {

  // Shutdown the server connection. Option 2 stops sending and receiving info.
  // If option 2 is a bit too harsh, try options 0 (stop receiving only) or
  // option 1 (stop transmitting and receiving ACKs only).
  if(shutdown(server_fd, 2) != 0) {
    logger.error("Failed to close server connection!");
    return -1;
  }

  if(close(server_fd) != 0) {
    logger.error("Failed to close server socket!");
    return -1;
  }

  running = false;

  return 0;

}
