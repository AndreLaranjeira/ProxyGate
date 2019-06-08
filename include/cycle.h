#ifndef CYCLE_H
#define CYCLE_H

// Library includes:
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdexcept>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// User includes:
#include "include/message_logger.h"
#include "include/socket.h"
#include "include/httpparser.h"

#define BUFFERSIZE 131072

typedef enum {
  AWAIT_CONNECTION,
  READ_FROM_CLIENT,
  SEND_TO_CLIENT,
  READ_FROM_WEBSITE,
  SEND_TO_WEBSITE,
  AWAIT_GATE
} CycleState;

class Cycle {
    public:
        Cycle(int server_fd, uint16_t port_number);
        ~Cycle();

        int execute();
    private:
        in_port_t port_number;
        int server_fd;
        MessageLogger logger;

        int await_connection();
        int read_from_client(int, char *);
        int connect_to_website(QString);
        int send_to_website(int , char *, ssize_t);
        int send_to_client(int , char *, ssize_t);
        int read_from_website(int, char *, size_t);

};

#endif // CYCLE_H
