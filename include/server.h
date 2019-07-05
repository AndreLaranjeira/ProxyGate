// Server module - Header file.

/**
 * @file server.h
 * @brief Server module - Header file.
 *
 * The server module contains the implementation of a proxy server class and
 * the definition of macros and data structures related to general server
 * functionalities and specific functions implemented by the proxy server
 * class. This header file contains a header guard, library includes, macro
 * definitions, type definitions and the class headers for this module.
 *
 */

// Header guard:
#ifndef SERVER_H
#define SERVER_H

// Library includes:
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdexcept>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// Qt includes:
#include <QMutex>
#include <QObject>
#include <QString>

// User includes:
#include "include/httpparser.h"
#include "include/message_logger.h"
#include "include/socket.h"

// Namespace:
using namespace std;

// Macros:

/**
 * @def DEFAULT_PORT
 * @brief Default port value utilized by the proxy server class.
 */

#define DEFAULT_PORT 8228

/**
 * @def SERVER_BACKLOG
 * @brief Number of backlog connections accepted by the proxy server class.
 */

#define SERVER_BACKLOG 3

// Type definitions:

/**
 * @enum ServerConnections
 * @brief Types of connections accepted by the proxy server.
 *
 * Enumeration of the types of connections the proxy server class can receive.
 * It is used to determine the next task performed by the proxy server after a
 * request from a certain connection is inspected or edited.
 *
 */

typedef enum {
  CLIENT,   /**< Client-side connection (web browser). */
  WEBSITE   /**< Connection to a website (proxy server-side). */
} ServerConnections;

/**
 * @enum ServerTask
 * @brief Types of tasks performed by the proxy server.
 *
 * Enumeration of the types of tasks performed by the proxy server class. It is
 * used in the run method of the proxy server class to implement a finite state
 * machine, used to determine the method the server class should call to
 * execute the next server task.
 *
 */

typedef enum {
  AWAIT_CONNECTION,     /**< Await for a client connection. */
  AWAIT_GATE,           /**< Await for the proxy gate to be opened. */
  CONNECT_TO_WEBSITE,   /**< Connect to a website host given by the client. */
  READ_FROM_CLIENT,     /**< Read data from the client. */
  READ_FROM_WEBSITE,    /**< Read data from a website. */
  SEND_TO_CLIENT,       /**< Send data to the client. */
  SEND_TO_WEBSITE,      /**< Send data to a website. */
  UPDATE_REQUESTS       /**< Update requests with the user edits. */
} ServerTask;

/**
 * @struct request
 * @brief HTTP request information obtained from a socket.
 *
 * Models the relevant data contained in a single HTTP request read from a
 * socket.
 *
 */

typedef struct {
  char content[HTTP_BUFFER_SIZE+1];   /**< Request contents. */
  ssize_t size;                       /**< Request size (Needed for binary
                                           data!). */
} request;

/**
 * @struct connection
 * @brief Socket connection information.
 *
 * Models the relevant data necessary to establish and use a connection to a
 * socket.
 *
 */

typedef struct {
  int fd;                   /**< File descriptor for the socket connection. */
  request buffer;           /**< Request struct to hold the data received from
                                 the connection. */
  struct sockaddr_in addr;  /**< Address information of the socket
                                 connection. */
} connection;

/**
 * @class Server
 * @brief Proxy server class.
 *
 * The main feature of this application, the Server class has the functionality
 * of a proxy server, intermediating a connection between a client and a
 * website host, and allowing the inspection and modification of the header and
 * data contained in a request made by the client or by a website host.
 *
 * The handling of client connections by the Server is implemented by a finite
 * state machine (FSM), which breaks the functionality of the Server into small
 * tasks.
 *
 * The port number on which the Server listens for client requests can be
 * configured on the class constructor.
 *
 */

// Class headers:
class Server : public QObject {
  Q_OBJECT

  public:
    // Class methods:
    Server(in_port_t);
    ~Server();

    // Methods:
    int init();
    void load_client_request(QString, QByteArray);
    void load_website_request(QString, QByteArray);
    void open_gate();

  public slots:
    void run();
    void stop();

  signals:
    void clientData(QString, QByteArray); /**< Signals a client request. */
    void error(QString err);    /**< Signals an error. */
    void finished();            /**< Signals the Server finished running. */
    void gateOpened();          /**< Signals the Server gate opened. */
    void logMessage(QString);   /**< Signals a log message. */
    void newHost(QString);      /**< Signals the connection to a new host. */
    void websiteData(QString, QByteArray); /**< Signals a website request. */

  private:
    // Variables:
    bool gate_closed;       /**< Variable to control the Server gate. */
    bool running;           /**< Variable to control the Server execution. */
    int server_fd;          /**< File descriptor of the Server socket. */
    in_port_t port_number;  /**< Port number used by the Server. */

    // Classes and custom types:
    HTTPParser parser;            /**< HTTPParser used by the Server. */
    MessageLogger logger;         /**< MessageLogger used by the Server. */
    QMutex gate_mutex;            /**< Mutex to the gate_closed variable. */
    QMutex run_mutex;             /**< Mutex to the gate_closed variable. */
    QByteArray new_client_data;   /**< New client request data. */
    QString new_client_headers;   /**< New client request headers. */
    QByteArray new_website_data;  /**< New website request data. */
    QString new_website_headers;  /**< New website request headers. */
    ServerConnections last_read;  /**< Last connection the server read from. */
    ServerTask next_task;         /**< Next server task to be executed. */

    // Methods:
    bool is_gate_closed();
    bool is_program_running();
    int await_connection(connection*);
    int await_gate();
    int connect_to_website(connection*, connection*);
    int execute_task(ServerTask, connection*, connection*);
    int read_from_client(connection*);
    int read_from_website(connection*);
    int send_to_client(connection*, connection*);
    int send_to_website(connection*, connection*);
    int update_requests(connection*, connection*);
    void config_client_addr(struct sockaddr_in*);
    void config_website_addr(struct sockaddr_in*);
    void handle_error(ServerTask, int, int);
    void replace_buffer(request *, QByteArray);
    void set_gate_closed(bool);
    void set_running(bool);

};

#endif // SERVER_H


