#include "include/spider.h"

Spider::Spider() : logger("Spider"){
}

void Spider::execute(QString host){
    QString request;
    logger.info("Entered spider, host: " + host.toStdString());

    if((get(host, &request)) < 0){
        logger.error("Unable to GET from website");
        return;
    }

    logger.info(request.toStdString());

    // Now extract links

}

int Spider::connect(QString host, int *website_fd){
    struct hostent *website_ip_data;
    struct sockaddr_in website_addr;

    website_addr.sin_family = AF_INET;
    website_addr.sin_addr.s_addr = INADDR_ANY;
    website_addr.sin_port = htons(80);

    // Asserts pointer is valid
    if(website_fd == nullptr) return -1;

    // Create socket
    *website_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(*website_fd == -1){
        logger.error("Failed to create server socket: " + string(strerror(errno)));
        return -1;
    }

    // Get IP address
    website_ip_data = gethostbyname(host.toStdString().c_str());
    if(website_ip_data == nullptr) {
        logger.error("Failed to find an IP address for the server website: " + host.toStdString());
        close(*website_fd);
        return -1;
    }

    memcpy(&(website_addr.sin_addr.s_addr), website_ip_data->h_addr,
           static_cast<size_t> (website_ip_data->h_length));

    // Connect socket
    if((connect_socket(*website_fd,
                       reinterpret_cast<struct sockaddr *> (&website_addr),
                       sizeof(website_addr))) < 0){
        logger.error("Failed to connect to the website: " + string(strerror(errno)));
        close(*website_fd);
        return -1;
    }

    return 0;
}

int Spider::get(QString host, QString *ret){
    size_t max_size = HTTP_BUFFER_SIZE;
    ssize_t single_read;
    ssize_t size_read;
    char buffer[HTTP_BUFFER_SIZE+1];
    int website_fd;
    int length;
    HTTPParser parser;
    HTTPParser finalParser;

    // Try to connect
    if((connect(host, &website_fd)) < 0){
        logger.error("Could not connect to website");
        return -1;
    }

    // Send GET request
    QString toSend = "GET / HTTP/1.1\r\nHost: " + host + "\r\nConnection: close\r\n\r\n";
    if((send(website_fd, toSend.toStdString().c_str(), static_cast<size_t>(toSend.size()), 0)) == -1){
        logger.error("Error while sending spider GET");
        close(website_fd);
        return -1;
    }

    // Read first headers:
    logger.info("Reading from website");
    size_read = read_socket(website_fd, buffer, max_size);
    parser.parseRequest(buffer, size_read);

    logger.info("Received " + parser.getCode().toStdString() + " " + parser.getDescription().toStdString() + " from website");

    // Read content-length:
    Headers headers = parser.getHeaders();
    if(headers.contains("Content-Length")){
        length = headers["Content-Length"].first().toInt();
        while(size_read < length){
            logger.info("Reading extra data from website [" + to_string(size_read) + "/" + to_string(length) + "]");
            single_read = read_socket(website_fd, buffer+size_read,
                                      max_size - static_cast<size_t> (size_read));

            if(single_read == -1) {
                logger.error("Failed to read from website: " + string(strerror(errno)));
                close(website_fd);
                return -1;
            }

            if(static_cast<size_t> (size_read) == max_size){
                logger.error("Request is greater than buffer! Giving up");
                close(website_fd);
                return -1;
            }

            size_read += single_read;

        }

    }

    else if(headers.contains("Transfer-Encoding")){
        if(headers["Transfer-Encoding"].first() == "chunked"){
            while(
                (single_read = read_socket(website_fd,
                                           buffer+size_read,
                                           max_size - static_cast<size_t> (size_read))) > 0
            ){
                logger.info("Reading extra data from website (chunked) [" + to_string(size_read) + "/" + to_string(max_size) + "]");
                size_read += single_read;
            }
        }
    }
    else{
        logger.warning("Website response doesn't contain Content-Length neither Transfer-Encoding");
    }

    finalParser.parseRequest(buffer, size_read);

    char *data = finalParser.getData();
    ssize_t size = finalParser.getDataSize();
    data[size] = '\0';
    *ret = QString(data);

    close(website_fd);

    return 0;


}
