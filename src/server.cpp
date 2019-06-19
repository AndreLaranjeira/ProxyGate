// Server module - Source code.

// Includes:
#include "include/server.h"

// Class methods:
Server::Server(in_port_t port_number) : port_number(port_number), logger("Server") {

  // Connect message loggers:
  connect(&logger, SIGNAL (sendMessage(QString)), this,
          SIGNAL (logMessage(QString)));
  connect(&parser, SIGNAL (logMessage(QString)), this,
          SIGNAL (logMessage(QString)));

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

  logger.success("Server initialized!");

  return 0;

}

void Server::load_client_request(QString new_request) {
  string aux = new_request.toStdString();
  adjust_line_endings(&aux);
  strcpy(new_client_request.content, aux.c_str());
  new_client_request.size = static_cast<ssize_t>(aux.size());
}

void Server::load_website_request(QString new_request) {
  string aux = new_request.toStdString();
  adjust_line_endings(&aux);
  strcpy(new_website_request.content, aux.c_str());
  new_website_request.size = static_cast<ssize_t>(aux.size());
}

void Server::open_gate() {
  set_gate_closed(false);
}

void Server::run() {

  connection client, website;
  unsigned int runtime_errors = 0;

  // Configure connection addresses:
  config_client_addr(&(client.addr));
  config_website_addr(&(website.addr));

  // Set control variables:
  set_gate_closed(true);
  set_running(true);
  next_task = AWAIT_CONNECTION;

  while(is_program_running())
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
  set_running(false);

  // Notify user:
  logger.info("Stop signal received. Shutting down!");

}

// Private methods:
bool Server::is_gate_closed() {
  bool aux;
  gate_mutex.lock();
  aux = gate_closed;
  gate_mutex.unlock();
  return aux;
}

bool Server::is_program_running() {
  bool aux;
  run_mutex.lock();
  aux = running;
  run_mutex.unlock();
  return aux;
}

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

  else if(!is_program_running()) {
    close_socket(client_fd);
    return 0;
  }

  else {
    logger.error("Failed to accept an incoming connection!");
    return -1;
  }

}

int Server::await_gate() {

  logger.info("Awaiting for gate to open!");

  // Wait for the gate to open or for the program to finish:
  while(is_gate_closed() and is_program_running()) {}

  // Signal that the gate actually opened:
  emit gateOpened();

  // Set the next task:
  next_task = UPDATE_REQUESTS;

  // Close the gates again:
  set_gate_closed(true);

  return 0;

}

int Server::connect_to_website(connection *client, connection *website){
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
    case UPDATE_REQUESTS:
      return_code = update_requests(client, website);
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
    emit clientRequest(QString(client->buffer));
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

    emit websiteRequest(QString(website->buffer));
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

int Server::update_requests(connection *client, connection *website){

  // Check which request we should update:
  switch(last_read) {

    case CLIENT:

      // If there were no edits, continue:
      if(strcmp(new_client_request.content, client->buffer) == 0) {
        next_task = CONNECT_TO_WEBSITE;
        logger.info("Client request unchanged!");
      }

      // If there were edits, we might need to overwrite the connection buffer:
      else {

        // If the new request is valid, overwrite the buffer:
        if (parser.parseRequest(new_client_request.content,
                                new_client_request.size)) {
          strcpy(client->buffer, new_client_request.content);
          client->buffer_size = new_client_request.size;
          next_task = CONNECT_TO_WEBSITE;
          logger.info("Edited client request!");
        }

        // Else, go back to the gate with the old request:
        else {
          emit clientRequest(QString(client->buffer));
          logger.error("Invalid client request entered! Try again!");
          next_task = AWAIT_GATE;
        }

      }

      break;

    case WEBSITE:

      // If there were no edits, continue:
      if(strcmp(new_website_request.content, website->buffer) == 0) {
        next_task = SEND_TO_CLIENT;
        logger.info("Website request unchanged!");
      }

      // If there were edits, we might need to overwrite the connection buffer:
      else {

        // If the new request is valid, overwrite the buffer:
        if (parser.parseRequest(new_website_request.content,
                                new_website_request.size)) {

          strcpy(website->buffer, new_website_request.content);
          website->buffer_size = new_website_request.size;
          logger.info("Edited website request!");
          next_task = SEND_TO_CLIENT;
        }

        // Else, go back to the gate with the old request:
        else {
          emit websiteRequest(QString(website->buffer));
          logger.error("Invalid client request entered! Try again!");
          next_task = AWAIT_GATE;
        }

      }

      break;

  }

  return 0;

}

void Server::adjust_line_endings(string *input) {
  size_t pos = 0, header_end;

  // Find the end of the header line:
  header_end = (*input).find("\n\n", 0);

  // Find an '\n' to replace:
  while ((pos = (*input).find("\n", pos)) != std::string::npos) {

    // If we find a '\n\n', we got to the header's end! Replace and leave:
    if(pos == header_end) {
      (*input).replace(pos, 2, "\r\n\r\n");
      break;
    }

    // Else, just replace the '\n':
    (*input).replace(pos, 1, "\r\n");
    pos += 2;
  }
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
    case UPDATE_REQUESTS:
      return;
  }

}

void Server::set_gate_closed(bool value) {
  gate_mutex.lock();
  gate_closed = value;
  gate_mutex.unlock();
}

void Server::set_running(bool value) {
  run_mutex.lock();
  running = value;
  run_mutex.unlock();
}
