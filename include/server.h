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
  SEND_TO_WEBSITE,
  UPDATE_REQUESTS
} ServerTask;

typedef struct {
  char content[HTTP_BUFFER_SIZE+1];
  ssize_t size;
} request;

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
    void load_client_request(QString);
    void load_website_request(QString);
    void open_gate();

  public slots:
    void run();
    void stop();

  signals:
    void clientRequest(QString req);
    void error(QString err);
    void finished();
    void gateOpened();
    void logMessage(QString);
    void websiteRequest(QString req);

  private:
    // Variables:
    bool gate_closed;
    bool running;
    int server_fd;
    in_port_t port_number;
    request new_client_request;
    request new_website_request;

    // Classes and custom types:
    HTTPParser parser;
    MessageLogger logger;
    QMutex gate_mutex;
    QMutex run_mutex;
    ServerConnections last_read;
    ServerTask next_task;

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
    void adjust_line_endings(string*);
    void config_client_addr(struct sockaddr_in*);
    void config_website_addr(struct sockaddr_in*);
    void handle_error(ServerTask, int, int);
    void set_gate_closed(bool);
    void set_running(bool);

};

#endif // SERVER_H


