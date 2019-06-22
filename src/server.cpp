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

void Server::load_client_header(QString new_headers, QByteArray new_data) {
  new_client_data = new_data;
  new_client_headers = new_headers;
  new_client_headers.replace('\n', "\r\n");   // Adjust line endings.
}

void Server::load_website_header(QString new_headers, QByteArray new_data) {
  new_website_data = new_data;
  new_website_headers = new_headers;
  new_website_headers.replace('\n', "\r\n");   // Adjust line endings.
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
  if(is_program_running())
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
    parser.parseRequest(client->buffer.content, client->buffer.size);

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
  if((client->buffer.size = read_socket(client->fd, client->buffer.content, HTTP_BUFFER_SIZE)) > 0) {

    parser.parseRequest(client->buffer.content, client->buffer.size);
    emit clientData(parser.requestHeaderToQString(), QByteArray(parser.getData(), parser.getDataSize()));
    emit newHost(parser.getHost());

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
    size_read = read_socket(website->fd, website->buffer.content, max_size);
    parser.parseRequest(website->buffer.content, size_read);

    logger.info("Received " + parser.getCode().toStdString() + " " + parser.getDescription().toStdString() + " from website");

    // Read content-length:
    Headers headers = parser.getHeaders();
    if(headers.contains("Content-Length")){
        length = headers["Content-Length"].first().toInt();
        while(size_read < length+parser.getHeadersSize()){
            logger.info("Reading extra data from website [" + to_string(size_read) + "/" + to_string(length) + "]");
            single_read = read_socket(website->fd, website->buffer.content+size_read,
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
                                           website->buffer.content+size_read,
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

    website->buffer.size = size_read;
    close(website->fd);

    parser.parseRequest(website->buffer.content, website->buffer.size);
    emit websiteData(parser.answerHeaderToQString(), QByteArray(parser.getData(), parser.getDataSize()));
    emit newHost(parser.getHost());

    last_read = WEBSITE;
    next_task = AWAIT_GATE;
    return 0;

}

int Server::send_to_client(connection *client, connection *website){
    logger.info("Sending message to client");

    if(send(client->fd, website->buffer.content, static_cast<size_t> (website->buffer.size), 0) == -1){
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

    if(send(website->fd, client->buffer.content,
            static_cast<size_t> (client->buffer.size), 0) == -1){
        logger.error("Failed to send: " + string(strerror(errno)));
        return -1;
    }

    logger.info("Sent some message to website!");
    next_task = READ_FROM_WEBSITE;
    return 0;

}

int Server::update_requests(connection *client, connection *website){

  QString full_request, original_header;
  QByteArray original_data;
  ssize_t body_size, original_size;
  QByteArray new_buffer;

  // Check which request we should update:
  switch(last_read) {

    case CLIENT:

      // Save original request data:
      parser.parseRequest(client->buffer.content, client->buffer.size);
      original_header = parser.requestHeaderToQString();
      original_data = QByteArray(parser.getData(), parser.getDataSize());
//      original_size = client->buffer.size;


      // If there were no edits, continue:
      if(new_client_headers == original_header && new_client_data == original_data) {
        next_task = CONNECT_TO_WEBSITE;
        logger.info("Client request unchanged!");
      }

      // If there were edits, we might need to overwrite the connection buffer:
      else {

        HTTPParser new_client_request;

        new_buffer.append(new_client_headers);
        new_buffer.append(new_client_data);

        // If the new request is valid, overwrite the buffer:
        if(new_client_request.parseRequest(new_buffer.data(), new_buffer.size())){
            new_client_request.updateContentLength();
            emit newHost(parser.getHost());
            replace_buffer(&(client->buffer), new_client_request.requestBuffer());
            next_task = CONNECT_TO_WEBSITE;
            logger.info("Edited client request!");
        }
        // Else, go back to the gate with the old request:
        else {
            emit clientData(parser.requestHeaderToQString(), QByteArray(parser.getData(), parser.getDataSize()));
            logger.error("Invalid client request entered! Try again!");
            next_task = AWAIT_GATE;
        }

//        // If the new request is valid, overwrite the buffer:
//        if (parser.validRequestHeader(new_client_data)) {
//          emit newHost(parser.getHost());
//          body_size = original_size - original_header.size();
//          replace_header(new_client_header, &(client->buffer), body_size);
//          next_task = CONNECT_TO_WEBSITE;
//          logger.info("Edited client request!");
//        }

//        // Else, go back to the gate with the old request:
//        else {
//          emit clientData(QByteArray(client->buffer.content, client->buffer.size));
//          logger.error("Invalid client request entered! Try again!");
//          next_task = AWAIT_GATE;
//        }

      }

      break;

    case WEBSITE:

      // Save original request data:
      parser.parseRequest(website->buffer.content, website->buffer.size);
      original_header = parser.requestHeaderToQString();
      original_data = QByteArray(parser.getData(), parser.getDataSize());

      // If there were no edits, continue:
      if(new_website_headers == original_header && new_website_data == original_data) {
        next_task = SEND_TO_CLIENT;
        logger.info("Website answer unchanged!");
      }

      // If there were edits, we might need to overwrite the connection buffer:
      else {

        HTTPParser new_website_answer;

        new_buffer.append(new_website_headers);
        new_buffer.append(new_website_data);

        // If the new request is valid, overwrite the buffer:
        if(new_website_answer.parseRequest(new_buffer.data(), new_buffer.size())){
            new_website_answer.updateContentLength();
            emit newHost(parser.getHost());
            replace_buffer(&(website->buffer), new_website_answer.answerBuffer());
            next_task = SEND_TO_CLIENT;
            logger.info("Edited website request!");
        }
        // Else, go back to the gate with the old request:
        else {
            emit websiteData(parser.answerHeaderToQString(), QByteArray(parser.getData(), parser.getDataSize()));
            logger.error("Invalid website answer entered! Try again!");
            next_task = AWAIT_GATE;
        }

      }

    break;

  }

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
    case UPDATE_REQUESTS:
      return;
  }

}

void Server::replace_header(QString new_header, request *req, ssize_t body_size) {
  char aux[HTTP_BUFFER_SIZE], *header_end;
  ssize_t new_size;

  // Find the new request size:
  new_size = new_header.size() + body_size;

  // Copy the new header:
  strcpy(aux, new_header.toStdString().c_str());

  // Find the end of the old header (the body beginning):
  header_end = strstr(req->content, "\r\n\r\n");
  header_end += 4;

  // Copy the old body:
  memcpy(aux + new_header.size(), header_end, static_cast <size_t> (body_size));

  // Copy the aux buffer to the request:
  memcpy(req->content, aux, static_cast<size_t> (new_size));

  // Update the size:
  req->size = new_size;

}

void Server::replace_buffer(request *req, QByteArray new_data){
    size_t size = static_cast<size_t> (new_data.size());
    if(size > HTTP_BUFFER_SIZE){
        logger.warning("Buffer is full");
        size = HTTP_BUFFER_SIZE;
    }
    memcpy(req->content, new_data.data(), size);
    req->size = new_data.size();
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
