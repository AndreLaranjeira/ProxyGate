#include "include/spider.h"

/**
 * @fn SpiderDumper::SpiderDumper()
 * @brief SpiderDumper constructor
 *
 * Instanciates and connects its logger to mainwindow
 */
SpiderDumper::SpiderDumper() : logger("SpiderDumper"){
    connect(&logger, SIGNAL (sendMessage(QString)), this,
            SIGNAL (updateLog(QString)));
}

/**
 * @fn void SpiderDumper::spider(QString link)
 * @brief Start spider tool
 * @param link Link to webpage to be root of spider
 *
 * This method creates a spider tree for the link given and emit
 * a signal to pretty print it on mainwindow
 */
void SpiderDumper::spider(QString link){
    QString request;

    emit updateSpiderTree("");

    logger.info("Entered spider");

    SpiderTree tree = buildSpiderTree(link, false);

    emit updateSpiderTree(tree.prettyPrint());

}

/**
 * @fn void SpiderDumper::dumper(QString link, QString dir)
 * @brief Start dumper tool
 * @param link Link to webpage to be root of dumper
 * @param dir Directory to save dump files
 *
 * This method creates a spider tree for the link given and saves each
 * file content in the tree, after that it fixes references and save each
 * file to dir folder
 */
void SpiderDumper::dumper(QString link, QString dir){
    QStringList links;

    logger.info("Entered dumper");

    SpiderTree tree = buildSpiderTree(link, true);

    dump(tree, dir);

    logger.info("Dump complete!");

}

/**
 * @fn SpiderTree::SpiderTree(QString link) : link(link)
 * @brief Constructor for SpiderTree
 * @param link The link of node
 */
SpiderTree::SpiderTree(QString link) : link(link){
}

/**
 * @fn void SpiderTree::appendNode(SpiderTree node)
 * @brief Append a node as child
 * @param node The node to be appended
 */
void SpiderTree::appendNode(SpiderTree node){
    this->nodes.push_back(node);
}

/**
 * @fn QString SpiderTree::prettyPrint()
 * @brief Pretty print spider tree
 * @return Returns a QString with pretty printed spider tree
 */
QString SpiderTree::prettyPrint(){
    return this->pp(0);
}

/**
 * @fn QString SpiderTree::pp(unsigned int level)
 * @brief Pretty print spider tree
 * @param level The actual level of print, used for tabulations
 * @return Returns a QString with pretty printed spider tree
 */
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

/**
 * @fn QByteArray SpiderTree::getData()
 * @brief Method that returns node's raw data
 * @return Returns raw data of node
 */
QByteArray SpiderTree::getData(){
    return this->data;
}

/**
 * @fn void SpiderTree::setData(QByteArray data)
 * @brief Set new data to node
 * @param data New data byte array
 */
void SpiderTree::setData(QByteArray data){
    this->data = data;
}

/**
 * @fn QString SpiderTree::getContentType()
 * @brief Method that returns node's raw data content type
 * @return Returns content type of node
 */
QString SpiderTree::getContentType(){
    return this->contentType;
}

/**
 * @fn void SpiderTree::setContentType(QString contentType)
 * @brief Method that sets node's raw data content type
 * @param contentType new content type to be set
 */
void SpiderTree::setContentType(QString contentType){
    this->contentType= contentType;
}

/**
 * @fn QString SpiderDumper::removeSquare(QString link)
 * @brief Removes '#' character from URL
 * @param link Link to remove character
 * @return Returns link with removed '#'
 */
QString SpiderDumper::removeSquare(QString link){
    return link.split("#")[0];
}

/**
 * @fn QString SpiderDumper::removeWWW(QString link)
 * @brief Removes 'www' string from URL
 * @param link Link to remove string
 * @return Returns link with removed 'www'
 */
QString SpiderDumper::removeWWW(QString link){
    return link.indexOf("www.") == 0 ? link.split("^www.")[0] : link;
}

/**
 * @fn bool SpiderDumper::sameHost(QString host, QString absoluteLink)
 * @brief Verifies if link belongs to same host
 * @param host The host to be compared
 * @param absoluteLink the link to verify if belongs to host
 * @return true if belongs, false if not
 */
bool SpiderDumper::sameHost(QString host, QString absoluteLink){
    return removeWWW(host) == getHost(removeWWW(absoluteLink));
}

/**
 * @fn list<SpiderTree> *SpiderTree::getNodes()
 * @brief Get all children nodes
 * @return List of nodes
 */
list<SpiderTree> *SpiderTree::getNodes(){
    return &(this->nodes);
}

/**
 * @fn QString SpiderTree::getLink()
 * @brief Get node link
 * @return QString with node link
 */
QString SpiderTree::getLink(){
    return this->link;
}

/**
 * @fn SpiderTree SpiderDumper::buildSpiderTree(QString link, bool dump)
 * @brief Method that creates spider tree given a link
 * @param link The link to be the root node
 * @param dump If true, saves each downloaded content in node and search for
 * other files rather than only links. If not don't save and only search for links
 * @return SpiderTree with SPIDER_TREE_DEPTH level of depth
 */
SpiderTree SpiderDumper::buildSpiderTree(QString link, bool dump){
    SpiderTree tree(getHost(link));
    QStringList links;
    links.append(getHost(link));

    buildSpiderTreeRecursive(&tree, link, getHost(link), SPIDER_TREE_DEPTH, &links, dump);

    return tree;
}

/**
 * @fn SpiderDumper::buildSpiderTreeRecursive(SpiderTree *tree, QString link, QString host, int depth, QStringList *globalLinks, bool dump)
 * @brief Recursive case for buildSpiderTree
 * @param tree Parent node to append new children
 * @param link The link of parent node
 * @param host The host to search for links and files
 * @param depth Build depth counter, if 0 than stop
 * @param globalLinks list of all links that have already been visited
 * @param dump Dump flag that enables saving raw data into node and search for more references rather than only hyperlinks
 * @return Answer from server for passed link
 */
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

/**
 * @fn QString SpiderDumper::getFileName(QString rawpath)
 * @brief Given a path to file get only file name
 * @param rawpath Path to file
 * @return File name
 */
QString SpiderDumper::getFileName(QString rawpath){
    // Split folder from filename
    size_t index = rawpath.toStdString().find_last_of("/");
    return QString::fromStdString(rawpath.toStdString().substr(index+1));
}

/**
 * @fn SpiderDumper::getFolderName(QString rawpath)
 * @brief Given a path to file get only folder name
 * @param rawpath Path to file
 * @return Folder to file
 */
QString SpiderDumper::getFolderName(QString rawpath){
    size_t index = rawpath.toStdString().find_last_of("/");
    return QString::fromStdString(rawpath.toStdString().substr(0,index+1));
}


/**
 * @fn QString SpiderDumper::getFileExtension(QString rawpath)
 * @brief Given a path to file get file extension
 * @param rawpath Path to file
 * @return File extension
 */
QString SpiderDumper::getFileExtension(QString rawpath){
    // Find for last .
    size_t index = rawpath.toStdString().find_last_of(".");

    // No . found, than no extension
    if(index == std::string::npos) return "";

    // Return extension
    return QString::fromStdString(rawpath.toStdString().substr(index+1));
}

/**
 * @fn void SpiderDumper::dump(SpiderTree tree, QString dir)
 * @brief Method that starts dump process
 * @param tree A tree with raw data to dump and fix references
 * @param dir Directory to save dumped files
 */
void SpiderDumper::dump(SpiderTree tree, QString dir){
    dumpRecursive(tree, dir);
}

/**
 * @fn SpiderDumper::saveToFile(QString folder, QString filename, QByteArray content)
 * @brief Method that saves a content to file
 * @param folder Folder to save file
 * @param filename Name of file to be saved
 * @param content Content of file
 */
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

/**
 * @fn SpiderDumper::dumpRecursive(SpiderTree node, QString dirPath)
 * @brief Recursive case for dump method, saves and fix references to links
 * @param node subtree to be dumped
 * @param dirPath path to save subtree files
 */
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

/**
 * @fn SpiderDumper::getAbsoluteLink(QString link, QString host)
 * @brief Format link
 * @param link Link to be formated
 * @param host Host to append if none
 * @return formated link
 *
 * Given a link remove http:// prefix, and try to add host as prefix if
 * it is not there. This method removes squares from links too
 */
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

/**
 * @fn QString SpiderDumper::getURL(QString link)
 * @brief Get URL from link
 * @param link Link to get URL
 * @return URL of link
 */
QString SpiderDumper::getURL(QString link){
    QRegularExpression re("(?:https?://)?(?:[^/]*)/*(.*)");
    QRegularExpressionMatch match = re.match(link);
    if(match.hasMatch()) return match.captured(1);
    else return link;
}

/**
 * @fn QString SpiderDumper::getURL_relative(QString link, QString url)
 * @brief Get URL relative to another URL
 * @param link Link to get relative url
 * @param url URL referential
 * @return Relative url of link
 *
 * Example if url is '/path/to/file.html' and link is '/path/other/file.html'
 * it should return '../../../path/other/file.html'
 */
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

/**
 * @fn QString SpiderDumper::getHost(QString link)
 * @brief Get host from link
 * @param link Link to get host url
 * @return Host as QString
 */
QString SpiderDumper::getHost(QString link){
    QRegularExpression re("(?:https?://)?([^/]*)/*(?:.*)");
    QRegularExpressionMatch match = re.match(link);
    if(match.hasMatch()) return match.captured(1);
    else return "";
}

/**
 * @fn QStringList SpiderDumper::extract_links(QString request)
 * @brief Extract links from raw data answer from website
 * @param request raw data answer from website
 * @return List of links as QStringList
 */
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

/**
 * @fn QStringList SpiderDumper::extract_references(QString request)
 * @brief Extract references from raw data answer from website
 * @param request raw data answer from website
 * @return List of references as QStringList
 *
 * References include javascript files, css files, images.
 */
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

/**
 * @fn QString SpiderDumper::buildBackDir(int backs)
 * @brief Given number of backs, return '../'xbacks
 * @param backs Number that represents number of backs in path
 * @return QString of '../'xbacks
 */
QString SpiderDumper::buildBackDir(int backs){
    QString ret;
    for(int i=0; i<backs ; i++)
        ret += "../";
    return ret;
}

/**
 * @fn QString SpiderDumper::fix_references(QString request, QString url)
 * @brief Fix references of website answer given reference url
 * @param request Answer as QString
 * @param url Url of document
 * @return Return reference-fixed answer
 */
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

/**
 * @fn int SpiderDumper::con(QString host, int *website_fd)
 * @brief Find IP address of host, creates socket and connects to website
 * @param host Host to connect
 * @return website_fd Socket file descriptor by reference
 * @return Return -1 if some error occurred
 */
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

/**
 * @fn int SpiderDumper::get(QString link, QByteArray *ret, QString *contentType)
 * @brief Given a link, sends a GET request to that link and returns its response by reference
 * @param link Link to send GET request
 * @return ret Array of bytes that will contain response (return by reference)
 * @return contentType The content type of response
 * @return Return -1 if some error occurred
 */
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
