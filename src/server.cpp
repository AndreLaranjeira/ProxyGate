// Server module - Source code.

// Includes:
#include "include/server.h"

// Class methods:
Server::Server(in_port_t port_number) : logger("Server") {

  // Configure the proxy client address:
  client_addr.sin_family = AF_INET;           // IPv4.
  client_addr.sin_addr.s_addr = INADDR_ANY;   // Accept all connections.
  client_addr.sin_port = htons(port_number);  // Port number for proxy.

  // Configure the proxy website server addresses:
  //memset(&website_addr, 0, sizeof(website_addr));

  //website_addr.sin_family = AF_INET;  // IPv4.
  //website_addr.sin_port = htons(80);  // Port number for website (HTTP).

  // Info message:
  logger.info("Server configured in port " + to_string(port_number) + ".");

}

Server::~Server() {

}

int Server::await_connection() {

  int addrlen = sizeof(client_addr);

  // Accept an incoming client connection (blocking function call):
  client_fd = accept(server_fd,
                     reinterpret_cast<struct sockaddr*> (&client_addr),
                     reinterpret_cast<socklen_t*> (&addrlen));

  if(client_fd > 0) {
    next_task = READ_FROM_CLIENT;
  }

  else if((client_fd < 0) && running) {
    logger.error("Failed to accept an incoming connection!");
    return -1;
  }

  return 0;

}

int Server::await_gate() {

//  // Wait for the gate to open or for the program to finish:
//  while(gate_closed) {
//    if(running == false)
//      break;
//  }

  // Decide next state based on the last read connection:
  switch(last_read) {

    case CLIENT:
      next_task = SEND_TO_WEBSITE;
      break;

    case WEBSITE:
      next_task = SEND_TO_CLIENT;
      break;

  }

  // Close the gates again:
  gate_closed = true;

  return 0;

}

int Server::execute_task(ServerTask task) {

  switch(task) {
    case AWAIT_CONNECTION:
      return await_connection();
    case READ_FROM_CLIENT:
      return read_from_client();
    case SEND_TO_CLIENT:
      return send_to_client();
    case READ_FROM_WEBSITE:
      return read_from_website();
    case SEND_TO_WEBSITE:
      return send_to_website();
    case AWAIT_GATE:
      return await_gate();
  }

  return -1;   // A failsafe!

}

int Server::read_from_client() {

  // Client sent data:
  if(read_socket(client_fd, client_buffer, BUFFERSIZE) == 0) {
    // Read from client_buffer to HTTPParser here.
    // If you want to create a method in the HTTPParser using read_socket to
    // read directly from the socket, that is also a good idea.
    //emit client_request(client_buffer); //(Notify MainWindow).
    cout << client_buffer << endl;

    http_parser = new HTTPParser();
    http_parser->parseRequest(QString(client_buffer));   // To determine host.

    last_read = CLIENT;
    next_task = AWAIT_GATE;
  }

  // Client has no more data to send:
  else
    next_task = AWAIT_CONNECTION;

  return 0;

}

int Server::read_from_website() {

  if(read_socket(website_fd, website_buffer, BUFFERSIZE) == 0) {
    // Read from website_buffer to HTTPParser here.
    // If you want to create a method in the HTTPParser using read_socket to
    // read directly from the socket, that is also a good idea.
    //emit website_request(website_buffer); //(Notify MainWindow).
    cout << website_buffer << endl;
    last_read = WEBSITE;
    next_task = AWAIT_GATE;
    return 0;
  }

  else {
    logger.error("Website did not send data!");
    next_task = SEND_TO_WEBSITE;  // Placeholder.
    return -1;
  }

}

int Server::send_to_client() {
  send(client_fd, website_buffer, BUFFERSIZE, 0);
  next_task = READ_FROM_CLIENT;
  return 0;
}

int Server::send_to_website() {
  struct sockaddr_in website_addr;

  if(http_parser == nullptr){
      next_task = READ_FROM_CLIENT;
      return -1;
  }

  // If it is the same host, send the package and don't ask questions.
  //if(last_host == http_parser->getHost().toStdString()) {
    //send(website_fd, client_buffer, BUFFERSIZE, 0);
    //logger.info("Sent some message!");
    //next_task = READ_FROM_WEBSITE;
    //return 0;
  //}

  close_socket(website_fd);
  website_fd = 0;
  if((website_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
      logger.error("Failed to create server socket!");
      return -1;
  }

  // Find the IP address for a given host:
  website_IP_data = gethostbyname(http_parser->getHost().toStdString().c_str());

  if(website_IP_data == nullptr) {
    logger.error("Failed to find an IP address for the server website");
    // Send a 404 page not found HTTP!
    // What should the next task be?
    // next_state = SEND_TO_CLIENT;
    return -1;
  }

  // Configure the proxy website server addresses:
  memset(&website_addr, 0, sizeof(website_addr));
  website_addr.sin_family = AF_INET;  // IPv4.
  website_addr.sin_port = htons(80);  // Port number for website (HTTP).


  // Copy the IP address obtained:
  memcpy(&website_addr.sin_addr.s_addr, website_IP_data->h_addr,
         static_cast<size_t> (website_IP_data->h_length));

  // Connect to the website:
  if(connect_socket(website_fd, reinterpret_cast<struct sockaddr *> (&website_addr), sizeof(website_addr)) < 0) {
    logger.error("Failed to connect to the website!");
    // Send a 403 connection refused HTTP!
    // What should the next task be?
     // next_state = SEND_TO_CLIENT;
    return -1;
  }

  else {
    // Send the request to the website:
    last_host = http_parser->getHost().toStdString();
    send(website_fd, client_buffer, BUFFERSIZE, 0);
    logger.info("Sent some message!");
    next_task = READ_FROM_WEBSITE;
    return 0;
  }

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

void Server::open_gate() {
  gate_closed = false;
}

void Server::run() {

  unsigned int runtime_errors = 0;

  gate_closed = true;
  running = true;
  next_task = AWAIT_CONNECTION;

  while(1)
    if(execute_task(next_task) != 0)
      runtime_errors++;

  logger.success("Server shutdown!");
  logger.info("Number of runtime errors: " + to_string(runtime_errors));

  emit finished();

}

void Server::stop() {

  // Close the connections:
  close_socket(server_fd);
  close_socket(website_fd);

  // Set a control variable
  running = false;

  // Notify user:
  logger.info("Stop signal received. Shutting down!");

}
