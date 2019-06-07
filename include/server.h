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
  READ_FROM_CLIENT,
  SEND_TO_CLIENT,
  READ_FROM_WEBSITE,
  SEND_TO_WEBSITE,
  AWAIT_GATE
} ServerTask;

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
    void website_request(QString req);

  private:
    // Variables:
    bool gate_closed;
    bool running;
    char client_buffer[8192];
    char website_buffer[8192];
    int client_fd;
    int server_fd;
    int website_fd;
    string last_host;
    struct hostent *website_IP_data;
    struct sockaddr_in client_addr;
    struct sockaddr_in website_addr;

    // Classes and custom types:
    HTTPParser http_parser = HTTPParser();
    MessageLogger logger;
    ServerConnections last_read;
    ServerTask next_task;

    // Methods:
    int await_connection();
    int await_gate();
    int execute_task(ServerTask);
    int read_from_client();
    int read_from_website();
    int send_to_client();
    int send_to_website();

};

#endif // SERVER_H
