#include "include/spider.h"

Spider::Spider() : logger("Spider"){
}

void Spider::execute(QString link){
    QString request;
    QStringList links;
    logger.info("Entered spider, host: " + link.toStdString());

    if((get(link, &request)) < 0){
        logger.error("Unable to GET from website");
        return;
    }

    links = extract_links(request);

    for(auto it = links.begin() ; it != links.end() ; ++it){
        logger.info(getAbsoluteLink((*it), getHost(link)).toStdString());
    }

}

QString Spider::getAbsoluteLink(QString link, QString host){
    QRegExp re("^http://(.*)");
    if(re.indexIn(link) == 0) return re.cap(1);
    else {
        if(link[0] == '/') return host + link;
        else return host + "/" + link;
    }
}

QString Spider::getURL(QString link){
    QRegExp re("(?:http:\\/\\/)?(?:[^/]*\\/*)?(.*)");
    if(re.indexIn(link) == 0) {
        if(re.cap(1).contains(".")) return "/";
        else return re.cap(1);
    }
    else return link;
}

QString Spider::getHost(QString link){
    QRegExp re("(?:http:\\/\\/)?(?:([^/]*)\\/*)?.*");
    if(re.indexIn(link) == 0) return re.cap(1);
    else return link;
}

QStringList Spider::extract_links(QString request){
    QStringList links;
    QRegularExpression re("href=[\"|']([^\"']*)[\"']");
    QRegularExpressionMatchIterator match = re.globalMatch(request);
    while(match.hasNext()){
        QString link = match.next().captured(1);
        if(!links.contains(link))
            links << link;
    }
    return links;
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

int Spider::get(QString link, QString *ret){
    size_t max_size = HTTP_BUFFER_SIZE;
    ssize_t single_read;
    ssize_t size_read;
    char buffer[HTTP_BUFFER_SIZE+1];
    int website_fd;
    int length;
    HTTPParser parser;
    HTTPParser finalParser;

    // Try to connect
    if((connect(getHost(link), &website_fd)) < 0){
        logger.error("Could not connect to website");
        return -1;
    }

    // Send GET request
    QString toSend = "GET " + getURL(link) + " HTTP/1.1\r\nHost: " + getHost(link) + "\r\nConnection: close\r\n\r\n";
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
