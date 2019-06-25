#include "include/spider.h"

SpiderDumper::SpiderDumper() : logger("SpiderDumper"){
    connect(&logger, SIGNAL (sendMessage(QString)), this,
            SIGNAL (updateLog(QString)));
}

void SpiderDumper::spider(QString link){
    QString request;
    QStringList links;

    emit updateSpiderTree("");

    logger.info("Entered spider");

    SpiderTree tree = buildSpiderTree(link, getHost(link), SPIDER_TREE_DEPTH, &links);

    emit updateSpiderTree(tree.prettyPrint());

}

void SpiderDumper::dumper(QString link, QString dir){
    QStringList links;

    logger.info("Entered dumper");

    dump(link, getHost(link), SPIDER_TREE_DEPTH, &links, dir);

    logger.info("Dump complete!");

}

SpiderTree::SpiderTree(QString link) : link(link){
}

void SpiderTree::appendNode(SpiderTree node){
    this->nodes.push_back(node);
}

QString SpiderTree::prettyPrint(){
    return this->pp(0);
}

QString SpiderTree::pp(unsigned int level){
    QString str;
    if(level == 0) str += "LINK: " + this->link + "\n";
    else {
        str += QString::fromStdString(std::string(4*level, ' ')) + this->link + "\n";
    }
    for (auto it = this->nodes.begin() ; it != this->nodes.end() ; ++it) {
        str += (*it).pp(level+1);
    }
    return str;
}

QString SpiderDumper::removeSquare(QString link){
    return link.split("#")[0];
}

QString SpiderDumper::removeWWW(QString link){
    return link.indexOf("www.") == 0 ? link.split("^www.")[0] : link;
}

bool SpiderDumper::sameHost(QString host, QString absoluteLink){
    return removeWWW(host) == getHost(removeWWW(absoluteLink));
}

SpiderTree SpiderDumper::buildSpiderTree(QString link, QString host, int depth, QStringList *globalLinks){
    QByteArray request;
    QStringList links;
    QString absoluteLink = getAbsoluteLink(link, getHost(link));
    SpiderTree tree(link);

    globalLinks->push_back(removeWWW(absoluteLink));

    logger.info("Entered SpiderTree builder, absolute link: " + absoluteLink.toStdString());

    if(depth == 0) return tree;

    if((get(link, &request)) < 0){
        logger.error("Unable to GET from website");
        return tree;
    }

    links = extract_links(request);

    for(auto it = links.begin() ; it != links.end() ; ++it){
        QString absoluteLink = getAbsoluteLink((*it), getHost(link));
        if(!globalLinks->contains(removeWWW(absoluteLink)) && sameHost(host, absoluteLink)){
            SpiderTree node = buildSpiderTree(absoluteLink, host, depth-1, globalLinks);
            tree.appendNode(node);
        }
    }

    return tree;
}

void SpiderDumper::dump(QString link, QString host, int depth, QStringList *globalLinks, QString dirpath){
    QByteArray request;
    QStringList links;
    QString absoluteLink = getAbsoluteLink(link, getHost(link));

    logger.info("Entered SpiderTree dumper builder, absolute link: " + absoluteLink.toStdString());

    globalLinks->push_back(removeWWW(absoluteLink));

    if(depth == 0) return;

    // Creates raw path
    std::string rawpath = dirpath.toStdString() + "/" + getURL(link).toStdString();

    // Split folder from filename
    size_t index = rawpath.find_last_of("/");
    std::string folder = rawpath.substr(0,index+1);
    std::string filename = rawpath.substr(index+1);

    // If filename is empty put index.html
    if(filename == ""){
        filename = "index.html";
    }

    // Creates intermediate folders
    QDir dir(QString::fromStdString(folder));
    if(!dir.exists()){
        dir.mkpath(".");
    }

    // Creates real path
    QString path = QString::fromStdString(folder) + QString::fromStdString(filename);

    // Opens file
    QFile file(path);
    if(!file.open(QIODevice::WriteOnly)){
        logger.error("Could not create file " + path.toStdString());
        return;
    }

    logger.info("Dump to " + path.toStdString());

    if((get(link, &request)) < 0){
        logger.error("Unable to GET from website");
        return;
    }

    // Saves to file
    file.write(request);
    file.close();

    links = extract_references(request);

    for(auto it = links.begin() ; it != links.end() ; ++it){
        QString absoluteLink = getAbsoluteLink((*it), getHost(link));
        if(!globalLinks->contains(removeWWW(absoluteLink)) && sameHost(host, absoluteLink)){
            dump(absoluteLink, host, depth-1, globalLinks, dirpath);
        }
    }

    return;
}

QString SpiderDumper::getAbsoluteLink(QString link, QString host){
    QRegExp re("^http(?:s)?://(.*)");
    QString ret;
    if(re.indexIn(link) == 0) ret = re.cap(1);
    else {
        if(link[0] == '/') ret = host + link;
        else if(removeWWW(link).indexOf(removeWWW(host)) == 0) ret = link;
        else ret = host + "/" + link;
    }
    return removeSquare(ret);
}

QString SpiderDumper::getURL(QString link){
    QRegularExpression re("(?:https?://)?(?:[^/]*)/*(.*)");
    QRegularExpressionMatch match = re.match(link);
    if(match.hasMatch()) return match.captured(1);
    else return link;
}

QString SpiderDumper::getHost(QString link){
    QRegularExpression re("(?:https?://)?([^/]*)/*(?:.*)");
    QRegularExpressionMatch match = re.match(link);
    if(match.hasMatch()) return match.captured(1);
    else return "";
}

QStringList SpiderDumper::extract_links(QString request){
    QStringList links;
    QRegularExpression re("<a.+?(?=href)href=[\"|']([^\"']*)[\"'][^>]*>");
    QRegularExpressionMatchIterator match = re.globalMatch(request);
    while(match.hasNext()){
        QString link = match.next().captured(1);
        if(!links.contains(link))
            links << link;
    }
    return links;
}

QStringList SpiderDumper::extract_references(QString request){
    QStringList links;
    QRegularExpression re("(?:href|src)=[\"|']([^\"']*)[\"']");
    QRegularExpressionMatchIterator match = re.globalMatch(request);
    while(match.hasNext()){
        QString link = match.next().captured(1);
        if(!links.contains(link))
            links << link;
    }
    return links;
}

int SpiderDumper::con(QString host, int *website_fd){
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

int SpiderDumper::get(QString link, QByteArray *ret){
    size_t max_size = HTTP_BUFFER_SIZE;
    ssize_t single_read;
    ssize_t size_read;
    char buffer[HTTP_BUFFER_SIZE+1];
    int website_fd;
    int length;
    HTTPParser parser;
    HTTPParser finalParser;

    // Try to connect
    if((con(getHost(link), &website_fd)) < 0){
        logger.error("Could not connect to website");
        return -1;
    }

    // Send GET request
    QString toSend = "GET /" + getURL(link) + " HTTP/1.1\r\nHost: " + getHost(link) + "\r\nConnection: close\r\n\r\n";
    if((send(website_fd, toSend.toStdString().c_str(), static_cast<size_t>(toSend.size()), 0)) == -1){
        logger.error("Error while sending spider GET");
        close(website_fd);
        return -1;
    }

    // Read first headers:
    //logger.info("Reading from website");
    size_read = read_socket(website_fd, buffer, max_size);
    parser.parseRequest(buffer, size_read);

    //logger.info("Received " + parser.getCode().toStdString() + " " + parser.getDescription().toStdString() + " from website");

    // Read content-length:
    Headers headers = parser.getHeaders();
    if(headers.contains("Content-Length")){
        length = headers["Content-Length"].first().toInt();
        while(size_read < length){
            //logger.info("Reading extra data from website [" + to_string(size_read) + "/" + to_string(length) + "]");
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
                //logger.info("Reading extra data from website (chunked) [" + to_string(size_read) + "/" + to_string(max_size) + "]");
                size_read += single_read;
            }
        }
    }
    else{
        logger.warning("Website response doesn't contain Content-Length neither Transfer-Encoding");
    }

    finalParser.parseRequest(buffer, size_read);

    *ret = QByteArray(finalParser.getData(), finalParser.getDataSize());

    close(website_fd);

    return 0;


}
