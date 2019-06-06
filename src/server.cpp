// Server module - Source code.

// Includes:
#include "include/server.h"

// Class methods:
Server::Server(in_port_t port_number) : logger("Server") {

  // Configure the proxy client address:
  client_address.sin_family = AF_INET;           // Internet address format.
  client_address.sin_addr.s_addr = INADDR_ANY;   // Accept connections from all IPs.
  client_address.sin_port = htons(port_number);  // Port number.

  // Configure the proxy website server addresses:
  memset(&website_address, 0, sizeof(website_address));

  website_address.sin_family = AF_INET;           // Internet address format.
  website_address.sin_port = htons(80);            // Port number.

  // Info message:
  logger.info("Server configured in port " + to_string(port_number) + ".");

}

Server::~Server() {

}

int Server::init() {

  // Variable declaration:
  int opt = 1;

  // Creating the proxy socket to listen to the client:
  if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    logger.error("Failed to create server socket!");
    return -1;
  }

  // Creating the proxy
  if((website_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
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
  if (bind(server_fd, reinterpret_cast<struct sockaddr*> (&client_address),
           sizeof(client_address)) != 0) {
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
  int addrlen = sizeof(client_address);

  running = true;

  while(running && !runtime_error) {
    // Accept an incoming client connection (blocking function call):
    client_fd = accept(server_fd,
                       reinterpret_cast<struct sockaddr*> (&client_address),
                       reinterpret_cast<socklen_t*> (&addrlen));

//    // Treating the client connection:
    // if(client_fd > 0) {
    //   ssize_t a = read(client_fd, request, 8192);
    //   request[a] = '\0';
    //
    //   HTTPParser parsedHTTP = HTTPParser();
    //   parsedHTTP.parseRequest(QString(request));
    //   parsedHTTP.prettyPrinter();
    //   //logger.info(parsedHTTP.toQString().toStdString());
//    }

//    // If we didn't ask the server to stop, then something went wrong!
//    else if(running) {
//      logger.error("Failed to accept an incoming connection!");
//      runtime_error = true;
//    }

    char hostname[] = "www.cplusplus.com";  // Obvious placeholder!

    // Find the IP address for a given host:
    website_IP_data = gethostbyname(hostname);

    if(website_IP_data == nullptr) {
      logger.error("Failed to find an IP address for the server website");
      runtime_error = 1;
      break;
    }

    memcpy(&website_address.sin_addr.s_addr, website_IP_data->h_addr, static_cast<size_t> (website_IP_data->h_length));

    // Connect to the website:
    if(connect_socket(website_fd, reinterpret_cast<struct sockaddr *> (&website_address), sizeof(website_address)) < 0) {
      logger.error("Failed to connect to the website!");
      runtime_error = true;
      break;
    }

    logger.success("Connected!");

    while(read(client_fd, client_request, 8192) > 0) {

      logger.info("Received some message!");
      cout << client_request << endl;

      // Send the request to the website:
      send(website_fd, client_request, 8192, 0);
      logger.info("Sent some message!");

      // Read the website's reply:
      read(website_fd, website_request, 8192);
      logger.info("Received some reply!");
      cout << website_request << endl;

      // Send the website's reply to the client:
      send(client_fd, website_request, 8192, 0);
      logger.info("Sent some reply!");

    }

    break;

  }

  while(running);

  if(!runtime_error)
    logger.success("Server shutdown!");

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
  shutdown(website_fd, 0);

  logger.info("Severed socket connections!");

  // Close the file descriptor:
  close(server_fd);
  close(website_fd);

  // Set a control variable
  running = false;

}
