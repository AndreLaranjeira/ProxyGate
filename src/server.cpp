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

  bool runtime_error = false;
  int addrlen = sizeof(address);

  running = true;

  while(running && !runtime_error) {
    // Accept an incoming client connection (blocking function call):
    client_fd = accept(server_fd,
                       reinterpret_cast<struct sockaddr*> (&address),
                       reinterpret_cast<socklen_t*> (&addrlen));

    // Treating the client connection:
    if(client_fd > 0) {
      ssize_t a = read(client_fd, request, 8192);
      request[a] = '\0';

      HTTPParser parsedHTTP = HTTPParser();
      parsedHTTP.parseRequest(QString(request));
      parsedHTTP.prettyPrinter();

    }

    // If we didn't ask the server to stop, then something went wrong!
    else if(running) {
      logger.error("Failed to accept an incoming connection!");
      runtime_error = true;
    }

  }

  if(!runtime_error)
    logger.success("Server successfully shutdown!");

  else
    logger.info("Server shutdown due to runtime error!");

  emit finished();

}

void Server::stop() {

  // Shutdown the server connection.
  // Argument options:
  //    0: Stop receiving data.
  //    1: Stop transmitting data and receiving ACKs.
  //    2: Stop receiving and transmitting data.
  shutdown(server_fd, 0);

  // Close the file descriptor:
  close(server_fd);

  // Set a control variable
  running = false;

}
