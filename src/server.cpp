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

  emit errorMessage("Server initialized!");

  logger.success("Server initialized!");

  return 0;

}

void Server::run() {

  unsigned int runtime_errors = 0;

  running = true;

  while(running){
      this->execute();
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


int Server::execute(){
    // Socket descriptors
    int client_fd;
    int website_fd;

    // Initialize buffer for client
    char client_buffer[BUFFERSIZE+1];
    ssize_t client_buffer_size;

    // Initialize buffer for website
    char website_buffer[BUFFERSIZE+1];
    ssize_t website_buffer_size;

    // Wait for client connection
    if((client_fd = await_connection()) <= 0){
        logger.error("Failed to accept an incoming connection!");
        return -1;
    }

    // Wait data from client
    if((client_buffer_size = read_socket(client_fd, client_buffer, BUFFERSIZE)) < 0){
        logger.error("No data read from client!");
        close_socket(client_fd);
        return -1;
    }

    // Parse client buffer
    HTTPParser client_parsed;
    client_parsed.parseRequest(client_buffer, client_buffer_size);
    logger.info("Client requested URL: " + client_parsed.getURL().toStdString());

    // Connect to website
    if((website_fd = connect_to_website(client_parsed.getHost())) <= 0){
        logger.error("Could not connect to website!");
        return -1;
    }

    // Send to website
    if(send_to_website(website_fd, client_buffer, client_buffer_size) == -1){
        logger.error("Failed send buffer to website!");
        close_socket(client_fd);
        close_socket(website_fd);
        return -1;
    }

    // Wait data from website
    if((website_buffer_size = read_from_website(website_fd, website_buffer, BUFFERSIZE)) == -1){
        logger.error("Failed to read from website!");
        close_socket(client_fd);
        close_socket(website_fd);
        return -1;
    }

    close_socket(website_fd);

    // Send to client
    if(send_to_client(client_fd, website_buffer, website_buffer_size) == -1){
        logger.error("Failed to send message to client!");
        close_socket(client_fd);
        return -1;
    }

    close_socket(client_fd);

    return 0;
}

int Server::await_connection() {

    struct sockaddr_in client_addr;

    client_addr.sin_family = AF_INET;           // IPv4.
    client_addr.sin_addr.s_addr = INADDR_ANY;   // Accept all connections.
    client_addr.sin_port = htons(port_number);  // Port number for proxy.

    int addrlen = sizeof(client_addr);

    // Accept an incoming client connection (blocking function call):
    logger.info("Waiting for connection from client");
    return accept(server_fd,
                     reinterpret_cast<struct sockaddr*> (&client_addr),
                     reinterpret_cast<socklen_t*> (&addrlen));

}

int Server::connect_to_website(QString host){
    int website_fd;
    struct hostent *website_IP_data;
    struct sockaddr_in website_addr;

    if((website_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        logger.error("Failed to create server socket!");
        return -1;
    }

    // Find the IP address for a given host:
    website_IP_data = gethostbyname(host.toStdString().c_str());

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
    logger.info("Connecting to website socket");
    if(connect_socket(website_fd, reinterpret_cast<struct sockaddr *> (&website_addr), sizeof(website_addr)) < 0) {
        logger.error("Failed to connect to the website!");
        // Send a 403 connection refused HTTP!
        // What should the next task be?
        // next_state = SEND_TO_CLIENT;
        return -1;
    }

    return website_fd;
}

int Server::send_to_website(int website_fd, char *send_buffer, ssize_t size){
    logger.info("Sending message to website");
    if(send(website_fd, send_buffer, (size_t)size, 0) == -1){
        logger.error("Failed to send: " + string(strerror(errno)));
        return -1;
    }

    logger.info("Sent some message to website!");
    return 0;
}


int Server::send_to_client(int client_fd, char *send_buffer, ssize_t size){
    logger.info("Sending message to client");
    if(send(client_fd, send_buffer, (size_t)size, 0) == -1){
        logger.error("Failed to send: " + string(strerror(errno)));
        return -1;
    }

    logger.info("Sent some message to client!");
    return 0;
}

int Server::read_from_website(int website_fd, char *website_buffer, size_t max_size){
    HTTPParser parser;
    ssize_t single_read;
    ssize_t size_read;

    // Read first headers
    logger.info("Reading from website");
    size_read = read_socket(website_fd, website_buffer, max_size);
    parser.parseRequest(website_buffer, size_read);

    logger.info("Received " + parser.getCode().toStdString() + " " + parser.getDescription().toStdString() + " from website");

    // Read content-length
    Headers headers = parser.getHeaders();
    if(headers.contains("Content-Length")){
        int length = headers["Content-Length"].first().toInt();
        while(size_read < length){
            logger.info("Reading extra data from website [" + to_string(size_read) + "/" + to_string(max_size) + "]");
            single_read = read_socket(website_fd, website_buffer+size_read, (ssize_t)max_size-size_read);
            if(single_read == -1){
                logger.error("Failed to read from website: " + string(strerror(errno)));
                return -1;
            }
            if((size_t)size_read == max_size){
                logger.error("Request is greater than buffer! Giving up");
                return -1;
            }
            size_read += single_read;
        }
    }

    return (int)size_read;
}

