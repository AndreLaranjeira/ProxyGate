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
    void run();
    void stop();

  signals:
    void client_request(QString req);
    void error(QString err);
    void finished();
    void website_request(QString req);
    void errorMessage(QString);

  private:
    // Variables:
    in_port_t port_number;

    bool running;
    int server_fd;

    // Classes and custom types:
    MessageLogger logger;

    // Methods:
    int execute();
    int await_connection();
    int connect_to_website(QString);
    int send_to_website(int, char *, ssize_t);
    int send_to_client(int, char *, ssize_t);
    int read_from_website(int, char *, size_t);

};

#endif // SERVER_H


