#include "include/spider.h"

SpiderDumper::SpiderDumper() : logger("SpiderDumper"){
    connect(&logger, SIGNAL (sendMessage(QString)), this,
            SIGNAL (updateLog(QString)));
}

void SpiderDumper::spider(QString link){
    QString request;

    emit updateSpiderTree("");

    logger.info("Entered spider");

    SpiderTree tree = buildSpiderTree(link, false);

    emit updateSpiderTree(tree.prettyPrint());

}

void SpiderDumper::dumper(QString link, QString dir){
    QStringList links;

    logger.info("Entered dumper");

    SpiderTree tree = buildSpiderTree(link, true);

    dump(tree, dir);

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

QByteArray SpiderTree::getData(){
    return this->data;
}

void SpiderTree::setData(QByteArray data){
    this->data = data;
}

QString SpiderTree::getContentType(){
    return this->contentType;
}

void SpiderTree::setContentType(QString contentType){
    this->contentType= contentType;
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

list<SpiderTree> *SpiderTree::getNodes(){
    return &(this->nodes);
}

QString SpiderTree::getLink(){
    return this->link;
}

SpiderTree SpiderDumper::buildSpiderTree(QString link, bool dump){
    SpiderTree tree(getHost(link));
    QStringList links;
    links.append(getHost(link));

    buildSpiderTreeRecursive(&tree, link, getHost(link), SPIDER_TREE_DEPTH, &links, dump);

    return tree;
}

QByteArray SpiderDumper::buildSpiderTreeRecursive(SpiderTree *tree, QString link, QString host, int depth, QStringList *globalLinks, bool dump){
    QByteArray request;
    QStringList links;
    QString contentType;
    QString absoluteLink = getAbsoluteLink(link, getHost(link));

    logger.info("Entered SpiderTree builder, absolute link: " + absoluteLink.toStdString());

    if(depth == 0) return "";

    if((get(link, &request, &contentType)) < 0){
        logger.error("Unable to GET from website");
        return "";
    }

    // Set content type to node
    (*tree).setContentType(contentType);

    if(dump){
        links = extract_references(request);
    }
    else {
        links = extract_links(request);
    }

    // Append child nodes
    for(auto it = links.begin() ; it != links.end() ; ++it){
        QString absoluteLink = getAbsoluteLink((*it), getHost(link));

        // Add only nodes that are in the same host and that haven't been added yet
        if(!globalLinks->contains(removeWWW(absoluteLink)) && sameHost(host, absoluteLink)){
            SpiderTree node(absoluteLink);
            globalLinks->push_back(removeWWW(absoluteLink));
            (*tree).appendNode(node);
        }
    }

    // Call for each child node recursivelly
    for(auto it = (*tree).getNodes()->begin() ; it != (*tree).getNodes()->end() ; ++it){
        QByteArray data = buildSpiderTreeRecursive(&(*it), (*it).getLink(), host, depth-1, globalLinks, dump);
        if(dump) (*it).setData(data);
    }

    return request;

}

QString SpiderDumper::getFileName(QString rawpath){
    // Split folder from filename
    size_t index = rawpath.toStdString().find_last_of("/");
    return QString::fromStdString(rawpath.toStdString().substr(index+1));
}

QString SpiderDumper::getFolderName(QString rawpath){
    size_t index = rawpath.toStdString().find_last_of("/");
    return QString::fromStdString(rawpath.toStdString().substr(0,index+1));
}

QString SpiderDumper::getFileExtension(QString rawpath){
    // Find for last .
    size_t index = rawpath.toStdString().find_last_of(".");

    // No . found, than no extension
    if(index == std::string::npos) return "";

    // Return extension
    return QString::fromStdString(rawpath.toStdString().substr(index+1));
}

void SpiderDumper::dump(SpiderTree tree, QString dir){
    dumpRecursive(tree, dir);
}

void SpiderDumper::saveToFile(QString folder, QString filename, QByteArray content){

    // Do not create empty files
    if(content.size() == 0) return;

    // Creates real path
    QString path = folder + filename;

    // Creates intermediate folders
    QDir dir(folder);
    if(!dir.exists()){
        dir.mkpath(".");
    }

    // Opens file
    QFile file(path);
    if(!file.open(QIODevice::WriteOnly)){
        logger.error("Could not create file " + path.toStdString());
        return;
    }

    file.write(content);
    file.close();
}

void SpiderDumper::dumpRecursive(SpiderTree node, QString dirPath){
    QString absoluteLink = getAbsoluteLink(node.getLink(), getHost(node.getLink()));
    QString request_replaced;
    QString contentType;

    logger.info("Entered SpiderTree dumper builder, absolute link: " + absoluteLink.toStdString());

    // Creates raw path
    QString rawpath = dirPath + "/" + getURL(node.getLink());

    // Split folder from filename
    QString folder = getFolderName(rawpath);
    QString filename = getFileName(rawpath);

    // If filename is empty put index.html
    if(filename == ""){
        filename = "index.html";
    }

    logger.info("Dump to " + (folder + filename).toStdString());

    // Fix references if html file and saves to file
    if(node.getContentType() == "text/html"){
        // If no extension but text/html, then we add .html
        if(getFileExtension(filename) == ""){
            filename += ".html";
        }
        request_replaced = fix_references(node.getData(), getURL(node.getLink()));

        saveToFile(folder, filename, request_replaced.toStdString().c_str());
    }

    // Do not fix references if not html file
    else {
        saveToFile(folder, filename, node.getData());
    }


    // Dump its children
    for(auto it = node.getNodes()->begin() ; it != node.getNodes()->end() ; ++it){
        dumpRecursive((*it), dirPath);
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

QString SpiderDumper::getURL_relative(QString link, QString url){
    QRegularExpression re("(?:https?://)?.*?(?=\\.\\.|/)/*(.*)");
    QRegularExpressionMatch match = re.match(link);

    if(getFileName(link) == ""){
        link = getFolderName(link) + "index.html";
    }

    // If no extension, then we add .html (how to know if its poiting to html content?)
    if(getFileExtension(link) == ""){
        link += ".html";
    }

    if(link.size() >= 1 && link[0] == "/"){
        return buildBackDir(url.count("/")) + link.mid(1);
    }
    else if(match.hasMatch()) return match.captured(1);
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
    QRegularExpression re("<a.+?(?=href)href\\s*=\\s*[\"|']([^\"']*)[\"'][^>]*>");
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
    QRegularExpression re("(?:href|src)\\s*=\\s*[\"|']([^\"']*)[\"']");
    QRegularExpressionMatchIterator match = re.globalMatch(request);
    while(match.hasNext()){
        QString link = match.next().captured(1);
        if(!links.contains(link))
            links << link;
    }
    return links;
}

QString SpiderDumper::buildBackDir(int backs){
    QString ret;
    for(int i=0; i<backs ; i++)
        ret += "../";
    return ret;
}

QString SpiderDumper::fix_references(QString request, QString url){
    QString ret = request;
    QRegularExpression re("(?:href|src)\\s*=\\s*[\"|']([^\"']*)[\"']");
    QRegularExpressionMatchIterator match_i = re.globalMatch(request);
    int offset = 0;
    while(match_i.hasNext()){
        QRegularExpressionMatch match = match_i.next();
        QString link = match.captured(1);
        QString fixed_link = getURL_relative(link, url);
        int match_begin = match.capturedStart(1);
        int match_end = match.capturedEnd(1);

        QString prefix = ret.mid(0, match_begin+offset);
        QString suffix = ret.mid(match_end+offset);

        offset += fixed_link.size() - link.size();

        ret = prefix + fixed_link + suffix;

    }
    return ret;
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

    // Configure timeout
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    if (setsockopt(*website_fd, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&tv), sizeof tv) != 0) {
      logger.error("Failed to configure server socket timeout!");
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

int SpiderDumper::get(QString link, QByteArray *ret, QString *contentType){
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
    if((size_read = read_socket(website_fd, buffer, max_size)) == -1){
        logger.error("Error while reading " + link.toStdString() + ": " + strerror(errno));
        close(website_fd);
        return -1;
    }
    parser.parseRequest(buffer, size_read);

    //logger.info("Received " + parser.getCode().toStdString() + " " + parser.getDescription().toStdString() + " from website");

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

    if(contentType != nullptr){
        Headers headers = finalParser.getHeaders();
        if(headers.contains("Content-Type")){
            *contentType = finalParser.getHeaders()["Content-Type"].first();
        }
        else *contentType = "";
    }

    return 0;


}
