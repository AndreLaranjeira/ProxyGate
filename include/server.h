// Server module - Header file.

// Header guard:
#ifndef SERVER_H
#define SERVER_H

// Library includes:
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

// Qt includes:
#include <QObject>

// User includes:
#include "include/message_logger.h"

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
    void error(QString err);
    void finished();

  private:
    // Variables:
    bool running = false;
    char request[8192];
    int client_fd;
    int server_fd;
    struct sockaddr_in address;

    // Classes:
    MessageLogger logger;

};

#endif // SERVER_H
