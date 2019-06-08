// Server module - Source code.

// Includes:
#include "include/server.h"

// Class methods:
Server::Server(in_port_t port_number) : port_number(port_number), logger("Server") {

  // Info message:
  logger.info("Server configured in port " + to_string(port_number) + ".");

}

Server::~Server() {

}

int Server::init() {

  // Variable declaration:
  int opt = 1;
  struct sockaddr_in client_addr;

  // Configure address on client side
  client_addr.sin_family = AF_INET;           // IPv4.
  client_addr.sin_addr.s_addr = INADDR_ANY;   // Accept all connections.
  client_addr.sin_port = htons(port_number);  // Port number for proxy.

  // Creating the proxy socket to listen to the client:
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
  if (bind(server_fd, reinterpret_cast<struct sockaddr*> (&client_addr),
           sizeof(client_addr)) != 0) {
    logger.error("Failed to bind server to the selected port!");
    return -1;
  }

  // Enable the server to listen to requests with a certain backlog:
  if(listen(server_fd, SERVER_BACKLOG) < 0) {
    logger.error("Failed configure socket to accept connections!");
    return -1;
  }

  // And there we go! This should make the server ready to begin accepting
  // requests. Just call Server::run() to begin.
  logger.success("Server initialized!");

  return 0;

}

void Server::run() {

  unsigned int runtime_errors = 0;

  running = true;

  while(running){
      Cycle *cycle = new Cycle(server_fd, port_number);
      cycle->execute();
      delete cycle;
  }

  logger.success("Server shutdown!");
  logger.info("Number of runtime errors: " + to_string(runtime_errors));

  emit finished();

}

void Server::stop() {

  // Close the connections:
  close_socket(server_fd);

  // Set a control variable
  running = false;

  // Notify user:
  logger.info("Stop signal received. Shutting down!");

}
