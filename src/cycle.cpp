// Cycle module - Source code.

// Includes:
#include "include/cycle.h"


Cycle::Cycle(int server_fd, uint16_t port_number) : port_number(port_number), server_fd(server_fd), logger("Cycle") {
}

Cycle::~Cycle(){
}

int Cycle::execute(){
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
    if((client_buffer_size = read_socket(client_fd, client_buffer, BUFFERSIZE)) == -1){
        logger.error("No data read from client!");
        close_socket(client_fd);
        return -1;
    }

    // Parse client buffer
    HTTPParser client_parsed;
    client_parsed.parseRequest(QString(client_buffer));

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

int Cycle::await_connection() {

    struct sockaddr_in client_addr;

    client_addr.sin_family = AF_INET;           // IPv4.
    client_addr.sin_addr.s_addr = INADDR_ANY;   // Accept all connections.
    client_addr.sin_port = htons(port_number);  // Port number for proxy.

    int addrlen = sizeof(client_addr);

    // Accept an incoming client connection (blocking function call):
    return accept(server_fd,
                     reinterpret_cast<struct sockaddr*> (&client_addr),
                     reinterpret_cast<socklen_t*> (&addrlen));

}

int Cycle::connect_to_website(QString host){
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
    if(connect_socket(website_fd, reinterpret_cast<struct sockaddr *> (&website_addr), sizeof(website_addr)) < 0) {
        logger.error("Failed to connect to the website!");
        // Send a 403 connection refused HTTP!
        // What should the next task be?
        // next_state = SEND_TO_CLIENT;
        return -1;
    }

    return website_fd;
}

int Cycle::send_to_website(int website_fd, char *send_buffer, ssize_t size){
    if(send(website_fd, send_buffer, (size_t)size, 0) == -1){
        logger.error("Failed to send: " + string(strerror(errno)));
        return -1;
    }

    logger.info("Sent some message to website!");
    return 0;
}


int Cycle::send_to_client(int client_fd, char *send_buffer, ssize_t size){
    if(send(client_fd, send_buffer, (size_t)size, 0) == -1){
        logger.error("Failed to send: " + string(strerror(errno)));
        return -1;
    }

    logger.info("Sent some message to client!");
    return 0;
}

int Cycle::read_from_website(int website_fd, char *website_buffer, size_t max_size){
    HTTPParser parser;
    ssize_t single_read;
    ssize_t size_read;

    // Read first headers
    size_read = read_socket(website_fd, website_buffer, max_size);
    parser.parseRequest(website_buffer);

    // Read content-length
    Headers headers = parser.getHeaders();
    if(headers.contains("Content-Length")){
        int length = headers["Content-Length"].first().toInt();
        while(size_read < length){
            single_read = read_socket(website_fd, website_buffer+size_read, (ssize_t)max_size-size_read);
            if(single_read == -1){
                logger.error("Failed to read from website: " + string(strerror(errno)));
                return -1;
            }
            size_read += single_read;
        }
    }

    return (int)size_read;
}
