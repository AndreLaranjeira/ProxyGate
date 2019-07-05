// Server module - Source code.

/**
 * @file server.cpp
 * @brief Server module - Source code.
 *
 * The server module contains the implementation of a proxy server class and
 * the definition of macros and data structures related to general server
 * functionalities and specific functions implemented by the proxy server
 * class. This source file contains the class method implementations for this
 * module.
 *
 */

// Includes:
#include "include/server.h"

// Class methods:

/**
 * @fn Server::Server(in_port_t port_number)
 * @brief Class constructor for the Server class.
 * @param port_number Local port number used by the proxy server.
 *
 * This constructor creates a new instance of the Server class. Each instance
 * has a port_number argument that configures the local port number used by the
 * server class.
 *
 * The server class contains an instance of a MessageLogger class
 * and an instance of a HTTPParser class, both of which have their message log
 * signal connected to the logMessage(QString) signal used by the server.
 *
 * This method logs a message with port number in which the server was
 * configured.
 *
 */

Server::Server(in_port_t port_number) : port_number(port_number), logger("Server") {

  // Connect message loggers:
  connect(&logger, SIGNAL (sendMessage(QString)), this,
          SIGNAL (logMessage(QString)));
  connect(&parser, SIGNAL (logMessage(QString)), this,
          SIGNAL (logMessage(QString)));

  // Info message:
  logger.info("Server configured in port " + to_string(port_number) + ".");

}

/**
 * @fn Server::~Server()
 * @brief Class destructor for the Server class.
 *
 * This destructor destroys an instance of the Server class. It is currently
 * empty!
 *
 */

Server::~Server() {

}

// Public methods:

/**
 * @fn int Server::init()
 * @brief Method to initialize the Server internal variables.
 * @return Returns 0 when the successfully executed and -1 if an error occurs.
 *
 * This method initializes the server socket used to handle client connections
 * and the client address information. You should always call this method
 * BEFORE atempting to run the server. If the server socket could not be
 * configured properly, this method will return -1 and log an error message
 * explaining what went wrong.
 *
 */

int Server::init() {

  // Variable declaration:
  int opt = 1;
  struct sockaddr_in client_addr;
  struct timeval tv;
  tv.tv_sec = 5;
  tv.tv_usec = 0;

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

  // Configure timeout
  if (setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&tv), sizeof tv) != 0) {
    logger.error("Failed to configure server socket timeout!");
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

/**
 * @fn void Server::load_client_request(QString new_headers, QByteArray new_data)
 * @brief Method to load an updated client request into the Server.
 * @param new_headers New headers for the client request.
 * @param new_data New data for the client request.
 *
 * This method is used to update the client request inside the Server to
 * reflect the changes made by an user. It is called from the MainWindow class
 * after an user edits the client request.
 *
 */

void Server::load_client_request(QString new_headers, QByteArray new_data) {
  new_client_data = new_data;
  new_client_headers = new_headers;
  new_client_headers.replace('\n', "\r\n");   // Adjust line endings.
}

/**
 * @fn void Server::load_website_request(QString new_headers, QByteArray new_data)
 * @brief Method to load an updated website request into the Server.
 * @param new_headers New headers for the website request.
 * @param new_data New data for the website request.
 *
 * This method is used to update the website request inside the Server to
 * reflect the changes made by an user. It is called from the MainWindow class
 * after an user edits the website request.
 *
 */

void Server::load_website_request(QString new_headers, QByteArray new_data) {
  new_website_data = new_data;
  new_website_headers = new_headers;
  new_website_headers.replace('\n', "\r\n");   // Adjust line endings.
}

/**
 * @fn void Server::open_gate()
 * @brief Method to open the Server gate.
 *
 * This method opens the Server gate, allowing a pending HTTP request from the
 * client or the website to be sent to a website or the client, respectively.
 *
 */

void Server::open_gate() {
  set_gate_closed(false);
}

/**
 * @fn void Server::run()
 * @brief Slot method for the Server to start handling client connections.
 *
 * This method is used for the Server to start accepting and handling client
 * connections. It is the main function of the Server, starting the execution
 * of server tasks and counting the number of runtime errors encountered during
 * the server execution.
 *
 * Calling the method stop() will stop the Server from executing new tasks,
 * ending this method. Once this method ends, the Server emits the finished()
 * signal, signaling it has finished it's execution and is ready to be deleted.
 *
 * Being a slot, the execution of this method can be connected to the arrival
 * of a signal.
 *
 * Important: You must call the init() method BEFORE calling this method!
 *
 */

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

/**
 * @fn void Server::stop()
 * @brief Slot method to stop the execution of the Server.
 *
 * This method is used to stop the execution of the Server. It closes the
 * Server socket allocated in the init() method and stops the execution of the
 * run() method, which will generate the emission of a finished() signal.
 *
 * Being a slot, the execution of this method can be connected to the arrival
 * of a signal
 *
 */

void Server::stop() {

  // Close the connections:
  close_socket(server_fd);

  // Set a control variable
  set_running(false);

  // Notify user:
  logger.info("Stop signal received. Shutting down!");

}

// Private methods:

/**
 * @fn bool Server::is_gate_closed()
 * @brief Method to check the value of the control variable gate_closed.
 * @return Value of the control variable gate_closed.
 *
 * This method returns the current value of the control variable gate_closed
 * using the QMutex gate_mutex.
 *
 */

bool Server::is_gate_closed() {
  bool aux;
  gate_mutex.lock();
  aux = gate_closed;
  gate_mutex.unlock();
  return aux;
}

/**
 * @fn bool Server::is_program_running()
 * @brief Method to check the value of the control variable 'running'.
 * @return Value of the control variable 'running'.
 *
 * This method returns the current value of the control variable 'running'
 * using the QMutex run_mutex.
 *
 */

bool Server::is_program_running() {
  bool aux;
  run_mutex.lock();
  aux = running;
  run_mutex.unlock();
  return aux;
}

/**
 * @fn int Server::await_connection(connection *client)
 * @brief Method used by the Server to wait for a client connection.
 * @param client Address of a struct to store the client connection info.
 * @return Returns 0 when the successfully executed and -1 if an error occurs.
 *
 * This method is used by the Server to wait for a client connection. Given
 * that this is the first task to be executed by the Server, an error caused in
 * others tasks usually leads back to this task.
 *
 * If this task is executed succesfully, the next task to be executed will be
 * READ_FROM_CLIENT.
 *
 */

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

/**
 * @fn int Server::await_gate()
 * @brief Method used by the Server to wait for the request gate to open.
 * @return Returns 0 when the successfully executed.
 *
 * This method is used by the Server to wait for the request gate to open.
 * While the gate does not open, the Server is stuck in busy waiting. When the
 * request gate is opened, the Server emits a gateOpened() signal and closes
 * the request gate again.
 *
 * If this task is executed successfully, the next task to be executed will be
 * UPDATE_REQUESTS.
 *
 * Important: If another thread or process does not call the open_gate()
 * method, the Server will be STUCK in busy waiting.
 *
 */

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

/**
 * @fn int Server::connect_to_website(connection *client, connection *website)
 * @brief Method used by the Server to connect to a website.
 * @param client Address of a struct to store the client connection info.
 * @param website Address of a struct to store the website connection info.
 * @return Returns 0 when the successfully executed and -1 if an error occurs.
 *
 * This method is used by the Server to connect to a website specified in a
 * client request. It creates a socket for the website connection, obtains the
 * website IP and tries to connect to it.
 *
 * If this task is executed succesfully, the next task to be executed will be
 * SEND_TO_WEBSITE.
 *
 */

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

/**
 * @fn int Server::execute_task(ServerTask task, connection *client, connection
 * *website)
 * @brief Method used by the Server to handle task execution.
 * @param task Task to be executed by the Server.
 * @param client Address of a struct to store the client connection info.
 * @param website Address of a struct to store the website connection info.
 * @return Returns the value returned by the task method executed.
 *
 * This method is used by the Server to figure out which method to execute
 * given a certain task. For each possible task, an underlying method call is
 * made with the proper parameters and the return code it generates is
 * returned by this function.
 *
 * If an underlying method call returns an error code, this method calls the
 * handle_error method and configures the next task to be executed to be
 * AWAIT_CONNECTION, reseting the finite state machine.
 *
 */

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

/**
 * @fn int Server::read_from_client(connection *client)
 * @brief Method used by the Server to read data from the client.
 * @param client Address of a struct to store the client connection info.
 * @return Returns 0 when the successfully executed and -1 if an error occurs.
 *
 * This method is used by the Server to read data from the client socket. It
 * stores the data read and it's size in the struct specified by the 'client'
 * parameter.
 *
 * If this task is executed succesfully, the next task to be executed will be
 * AWAIT_GATE, the last_read control variable is set to CLIENT and the signals
 * clientData(QString) and newHost(QString) are emitted, specifying the data
 * read from the client and the host in the client request.
 *
 */

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

/**
 * @fn int Server::read_from_website(connection *website)
 * @brief Method used by the Server to read data from the client.
 * @param website Address of a struct to store the website connection info.
 * @return Returns 0 when the successfully executed and -1 if an error occurs.
 *
 * This method is used by the Server to read data from the website socket. It
 * stores the data read and it's size in the struct specified by the 'website'
 * parameter. This method behaves differently depending on the header values
 * read from the website request.
 *
 * If this task is executed succesfully, the next task to be executed will be
 * AWAIT_GATE, the last_read control variable is set to WEBSITE and the signals
 * websiteData(QString) and newHost(QString) are emitted, specifying the data
 * read from the website and the host in the website request. Also, the success
 * of this method causes the website socket to be closed.
 *
 */

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

/**
 * @fn int Server::send_to_client(connection *client, connection *website)
 * @brief Method used by the Server to send data to the client.
 * @param client Address of a struct to store the client connection info.
 * @param website Address of a struct to store the website connection info.
 * @return Returns 0 when the successfully executed and -1 if an error occurs.
 *
 * This method is used by the Server to send data to the client socket. The
 * data is taken from the 'website' struct pointer.
 *
 * If this task is executed succesfully, the next task to be executed will be
 * AWAIT_CONNECTION and the client socket is closed.
 *
 */

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

/**
 * @fn int Server::send_to_website(connection *client, connection *website)
 * @brief Method used by the Server to send data to the website.
 * @param client Address of a struct to store the client connection info.
 * @param website Address of a struct to store the website connection info.
 * @return Returns 0 when the successfully executed and -1 if an error occurs.
 *
 * This method is used by the Server to send data to the website socket. The
 * data is taken from the 'client' struct pointer.
 *
 * If this task is executed succesfully, the next task to be executed will be
 * READ_FROM_WEBSITE.
 *
 */

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

/**
 * @fn int Server::update_requests(connection *client, connection *website)
 * @brief Method used by the Server to update requests based on user edits.
 * @param client Address of a struct to store the client connection info.
 * @param website Address of a struct to store the website connection info.
 * @return Returns 0 when the successfully executed.
 *
 * This method is used by the Server to update the header and data contained in
 * both the client and website requests based on the modifications made by the
 * user. If no modifications are made, the method just updates the next_task
 * control variable. If modifications were made, the method checks if the new
 * HTTP request produced is valid or not.
 *
 * If this task is executed succesfully, the next task to be executed will be
 * CONNECT_TO_WEBSITE if the last_read variable is CLIENT and SEND_TO_CLIENT if
 * the last_read variable is WEBSITE.
 *
 * Important: If the request edits made by the user result in a INVALID HTTP
 * request, the user is notified by an error log message, the ORIGINAL request
 * is restored and the next task to be executed is AWAIT_GATE, taking the user
 * back to the request gate.
 *
 */

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

      }

      break;

    case WEBSITE:

      // Save original request data:
      parser.parseRequest(website->buffer.content, website->buffer.size);
      original_header = parser.answerHeaderToQString();
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

/**
 * @fn Server::config_client_addr(struct sockaddr_in *client_addr)
 * @brief Method to configure the client socket address information.
 * @param client_addr Address of a struct to hold the information.
 *
 * This method configures the information used to connect to the client socket.
 * The information is saved in the struct pointer client_addr.
 *
 */

void Server::config_client_addr(struct sockaddr_in *client_addr) {
  client_addr->sin_family = AF_INET;           // IPv4.
  client_addr->sin_addr.s_addr = INADDR_ANY;   // Accept all connections.
  client_addr->sin_port = htons(port_number);  // Port number for proxy.
}

/**
 * @fn void Server::config_website_addr(struct sockaddr_in *website_addr)
 * @brief Method to configure the website socket address information.
 * @param website_addr Address of a struct to hold the information.
 *
 * This method configures the information used to connect to the website
 * socket. The information is saved in the struct pointer website_addr.
 *
 */

void Server::config_website_addr(struct sockaddr_in *website_addr) {
  memset(website_addr, 0, sizeof(*website_addr));
  website_addr->sin_family = AF_INET;  // IPv4.
  website_addr->sin_port = htons(80);  // Port number for website (HTTP).
}

/**
 * @fn void Server::handle_error(ServerTask task, int client_fd, int
 * website_fd)
 * @brief Method to handle errors associated with a Server task.
 * @param task Server task that cause an error.
 * @param client_fd File descriptor for the client socket connection.
 * @param website_fd File descriptor for the website socket connection.
 *
 * This method is used to take the necessary actions when a task performed by
 * the Server run method causes an error. Sometimes, no actions need to be
 * taken!
 *
 */

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

/**
 * @fn void Server::replace_buffer(request *req, QByteArray new_data)
 * @brief Method to replace the content and size of a request data type.
 * @param req Address of the request whose content and size will be replaced.
 * @param new_data Data to be written to the request.
 *
 * This method replaces the content of a request specified by the address req
 * with the data provided by new_data. It also changes the size specified in
 * req to that of the data in new_data.
 *
 * Warning: The old content and size of the request specified by the address
 * req will be OVERWRITTEN.
 *
 */

void Server::replace_buffer(request *req, QByteArray new_data){
    size_t size = static_cast<size_t> (new_data.size());
    if(size > HTTP_BUFFER_SIZE){
        logger.warning("Buffer is full");
        size = HTTP_BUFFER_SIZE;
    }
    memcpy(req->content, new_data.data(), size);
    req->size = new_data.size();
}

/**
 * @fn void Server::set_gate_closed(bool value)
 * @brief Method to set the value of the gate_closed control variable.
 * @param value Value to be assigned to the gate_closed control variable.
 *
 * This method sets the value of the gate_closed control variable using the
 * QMutex gate_mutex.
 *
 */

void Server::set_gate_closed(bool value) {
  gate_mutex.lock();
  gate_closed = value;
  gate_mutex.unlock();
}

/**
 * @fn void Server::set_running(bool value)
 * @brief Method to set the value of the 'running' control variable.
 * @param value Value to be assigned to the 'running' control variable.
 *
 * This method sets the value of the 'running' control variable using the
 * QMutex run_mutex.
 *
 */

void Server::set_running(bool value) {
  run_mutex.lock();
  running = value;
  run_mutex.unlock();
}
