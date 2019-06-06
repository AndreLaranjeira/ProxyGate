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
    void error(QString err);
    void finished();

  private:
    // Variables:
    bool running = false;
    char client_request[8192];
    char website_request[8192];
    int client_fd;
    int server_fd;
    int website_fd;
    struct hostent *website_IP_data;
    struct sockaddr_in client_address;
    struct sockaddr_in website_address;

    // Classes:
    MessageLogger logger;

};

#endif // SERVER_H
