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

// Public methods:
int Server::init() {

  // Variable declaration:
  int opt = 1;
  struct sockaddr_in client_addr;

  // Configure address on client side
  config_client_addr(&client_addr);

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

  emit errorMessage("Server initialized!");

  logger.success("Server initialized!");

  return 0;

}

void Server::run() {

  connection client, website;
  unsigned int runtime_errors = 0;

  // Configure connection addresses:
  config_client_addr(&(client.addr));
  config_website_addr(&(website.addr));

  // Set control variables:
  running = true;
  next_task = AWAIT_CONNECTION;

  while(running)
    if(execute_task(next_task, &client, &website) != 0)
      runtime_errors++;

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

// Private methods:
int Server::await_connection(connection *client) {

  int addrlen = sizeof(client->addr), client_fd;

  // Accept an incoming client connection (blocking function call):
  logger.info("Waiting for connection from client");

  client_fd = accept(server_fd,
                     reinterpret_cast<struct sockaddr*> (&(client->addr)),
                     reinterpret_cast<socklen_t*> (&addrlen));

  // Check to see if the connection succeded:
  if(client_fd != -1) {
    client->fd = client_fd;
    next_task = READ_FROM_CLIENT;
    return 0;
  }

  else if(!running) {
    close_socket(client_fd);
    return 0;
  }

  else {
    logger.error("Failed to accept an incoming connection!");
    return -1;
  }

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
      next_task = CONNECT_TO_WEBSITE;
      break;

    case WEBSITE:
      next_task = SEND_TO_CLIENT;
      break;

  }

  // Close the gates again:
  // gate_closed = true;

  return 0;

}

int Server::connect_to_website(connection *client, connection *website){
    HTTPParser parser;
    struct hostent *website_IP_data;

    if((website->fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        logger.error("Failed to create server socket!");
        return -1;
    }

    // Find the host name from the client request:
    parser.parseRequest(client->buffer, client->buffer_size);

    // Find the IP address for a given host:
    website_IP_data = gethostbyname(parser.getHost().toStdString().c_str());

    if(website_IP_data == nullptr) {
        logger.error("Failed to find an IP address for the server website");
        close(website->fd);
        return -1;
    }

    // Copy the IP address obtained:
    memcpy(&(website->addr.sin_addr.s_addr), website_IP_data->h_addr,
           static_cast<size_t> (website_IP_data->h_length));

    // Connect to the website:
    logger.info("Connecting to website socket");
    if(connect_socket(website->fd,
                      reinterpret_cast<struct sockaddr *> (&(website->addr)),
                      sizeof(website->addr)) < 0) {
        logger.error("Failed to connect to the website!");
        close(website->fd);
        return -1;
    }

    next_task = SEND_TO_WEBSITE;
    return 0;

}

int Server::execute_task(ServerTask task, connection *client,
                         connection *website) {

  int return_code = -1; // A failsafe (in case the switch fails)!

  switch(task) {
    case AWAIT_CONNECTION:
      return_code = await_connection(client);
      break;
    case AWAIT_GATE:
      return_code = await_gate();
      break;
    case CONNECT_TO_WEBSITE:
      return_code = connect_to_website(client, website);
      break;
    case READ_FROM_CLIENT:
      return_code = read_from_client(client);
      break;
    case READ_FROM_WEBSITE:
      return_code = read_from_website(website);
      break;
    case SEND_TO_CLIENT:
      return_code = send_to_client(client, website);
      break;
    case SEND_TO_WEBSITE:
      return_code = send_to_website(client, website);
      break;
  }

  // In case an error occurred:
  if(return_code == -1) {
    handle_error(task, client->fd, website->fd);  // Take necessary actions.
    next_task = AWAIT_CONNECTION;                 // Reset the event loop;
  }

  return return_code;

}

int Server::read_from_client(connection *client) {

  // Client sent data:
  if((client->buffer_size = read_socket(client->fd, client->buffer, HTTP_BUFFER_SIZE)) > 0) {
    last_read = CLIENT;
    next_task = AWAIT_GATE;
    return 0;
  }

  // Client didn't send data:
  else {
    logger.error("No data read from client!");
    return -1;
  }

}

int Server::read_from_website(connection *website){

    HTTPParser parser;
    int length;
    size_t max_size = HTTP_BUFFER_SIZE;
    ssize_t single_read;
    ssize_t size_read;

    // Read first headers:
    logger.info("Reading from website");
    size_read = read_socket(website->fd, website->buffer, max_size);
    parser.parseRequest(website->buffer, size_read);

    logger.info("Received " + parser.getCode().toStdString() + " " + parser.getDescription().toStdString() + " from website");

    // Read content-length:
    Headers headers = parser.getHeaders();
    if(headers.contains("Content-Length")){
        length = headers["Content-Length"].first().toInt();
        while(size_read < length){
            logger.info("Reading extra data from website [" + to_string(size_read) + "/" + to_string(length) + "]");
            single_read = read_socket(website->fd, website->buffer+size_read,
                                      static_cast<ssize_t> (max_size-size_read));

            if(single_read == -1) {
                logger.error("Failed to read from website: " + string(strerror(errno)));
                return -1;
            }

            if(static_cast<size_t> (size_read) == max_size){
                logger.error("Request is greater than buffer! Giving up");
                return -1;
            }

            size_read += single_read;

        }

    }

    else if(headers.contains("Transfer-Encoding")){
        if(headers["Transfer-Encoding"].first() == "chunked"){
            while(
                (single_read = read_socket(website->fd,
                                           website->buffer+size_read,
                                           static_cast<ssize_t> (max_size-size_read))) > 0
            ){
                logger.info("Reading extra data from website (chunked) [" + to_string(size_read) + "/" + to_string(max_size) + "]");
                size_read += single_read;
            }
        }
    }
    else{
        logger.warning("Website response doesn't contain Content-Length neither Transfer-Encoding");
    }

    website->buffer_size = size_read;
    close(website->fd);

    last_read = WEBSITE;
    next_task = AWAIT_GATE;
    return 0;

}

int Server::send_to_client(connection *client, connection *website){
    logger.info("Sending message to client");

    if(send(client->fd, website->buffer, static_cast<size_t> (website->buffer_size), 0) == -1){
        logger.error("Failed to send: " + string(strerror(errno)));
        return -1;
    }

    logger.info("Sent some message to client!");
    close(client->fd);
    next_task = AWAIT_CONNECTION;
    return 0;

}

int Server::send_to_website(connection *client, connection *website){

    logger.info("Sending message to website");

    if(send(website->fd, client->buffer,
            static_cast<size_t> (client->buffer_size), 0) == -1){
        logger.error("Failed to send: " + string(strerror(errno)));
        return -1;
    }

    logger.info("Sent some message to website!");
    next_task = READ_FROM_WEBSITE;
    return 0;

}

void Server::config_client_addr(struct sockaddr_in *client_addr) {
  client_addr->sin_family = AF_INET;           // IPv4.
  client_addr->sin_addr.s_addr = INADDR_ANY;   // Accept all connections.
  client_addr->sin_port = htons(port_number);  // Port number for proxy.
}

void Server::config_website_addr(struct sockaddr_in *website_addr) {
  memset(website_addr, 0, sizeof(*website_addr));
  website_addr->sin_family = AF_INET;  // IPv4.
  website_addr->sin_port = htons(80);  // Port number for website (HTTP).
}

void Server::handle_error(ServerTask task, int client_fd, int website_fd) {

  switch(task) {
    case AWAIT_CONNECTION:
      return;
    case AWAIT_GATE:
      return;
    case CONNECT_TO_WEBSITE:
      close(client_fd);
      break;
    case READ_FROM_CLIENT:
      close(client_fd);
      break;
    case READ_FROM_WEBSITE:
      close(client_fd);
      close(website_fd);
      break;
    case SEND_TO_CLIENT:
      close(client_fd);
      break;
    case SEND_TO_WEBSITE:
      close(client_fd);
      close(website_fd);
      break;
  }

}
