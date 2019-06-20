// Server module - Header file.

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
#include <QObject>

// User includes:
#include "include/httpparser.h"
#include "include/message_logger.h"
#include "include/socket.h"

// Namespace:
using namespace std;

// Macros:
#define DEFAULT_PORT 8228
#define SERVER_BACKLOG 3

// Type definitions:
typedef enum {
  CLIENT,
  WEBSITE
} ServerConnections;

typedef enum {
  AWAIT_CONNECTION,
  AWAIT_GATE,
  CONNECT_TO_WEBSITE,
  READ_FROM_CLIENT,
  READ_FROM_WEBSITE,
  SEND_TO_CLIENT,
  SEND_TO_WEBSITE
} ServerTask;

typedef struct {
  int fd;
  char buffer[HTTP_BUFFER_SIZE+1];
  ssize_t buffer_size;
  struct sockaddr_in addr;
} connection;

// Class headers:
class Server : public QObject {
  Q_OBJECT

  public:
    // Class methods:
    Server(in_port_t);
    ~Server();

    // Methods:
    int init();

  public slots:
    void open_gate();
    void run();
    void stop();

  signals:
    void client_request(QString req);
    void error(QString err);
    void finished();
    void gate_opened();
    void logMessage(QString);
    void website_request(QString req);

  private:
    // Variables:
    in_port_t port_number;

    bool gate_closed;
    bool running;
    int server_fd;

    // Classes and custom types:
    HTTPParser parser;
    MessageLogger logger;
    ServerConnections last_read;
    ServerTask next_task;

    // Methods:
    int await_connection(connection*);
    int await_gate();
    int connect_to_website(connection*, connection*);
    int execute_task(ServerTask, connection*, connection*);
    int read_from_client(connection*);
    int read_from_website(connection*);
    int send_to_client(connection*, connection*);
    int send_to_website(connection*, connection*);
    void config_client_addr(struct sockaddr_in*);
    void config_website_addr(struct sockaddr_in*);
    void handle_error(ServerTask, int, int);

};

#endif // SERVER_H


