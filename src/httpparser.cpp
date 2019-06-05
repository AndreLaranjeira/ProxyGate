#include "include/httpparser.h"


HTTPParser::HTTPParser() : logger("HTTPParser"){
    this->state = COMMANDLINE;
    this->method = "";
    this->url = "";
    this->version = "";
    this->data = "";
}

bool HTTPParser::parse(QString request){
    QList<QString> lines;

    lines = request.split(QRegExp("\r\n"));

    for (QString line : lines) {

        switch (state) {
            case COMMANDLINE:
                if(!this->parseCommandLine(line)){
                   logger.error("Could not parse COMMAND LINE");
                   return false;
                }
                state = HEADERLINE;
            break;
            case HEADERLINE:
                if(line == ""){
                    state = DATA;
                }
                else {
                    if(!this->parseHeaderLine(line)){
                        logger.error("Could not parse HEADER LINE");
                        return false;
                    }
                }
            break;
            case DATA:
                this->data += line;
            break;
        }
    }

    logger.info("Successfully parsed HTTP request");

    return true;
}

bool HTTPParser::parseCommandLine(QString line){
    int pos;
    QRegExp commandline("^(GET|HEAD|CONNECT|PUT|DELETE|POST|OPTIONS|TRACE|PATCH) ([:|\\/\\.\\\\-\\w_~/?\\#\\[\\]@!\\$\\^&'\\(\\)\\*\\+\\´,;=%{}]+) (HTTP\\/(?:\\d.\\d))$");
    QString method, url, version;

    pos = commandline.indexIn(line);
    if(pos != 0) return false;

    this->method = commandline.cap(1);
    this->url = commandline.cap(2);
    this->version = commandline.cap(3);

    return true;
}

bool HTTPParser::parseHeaderLine(QString line){
    int pos;
    QRegExp headerline("^([A-Za-z-]*): (.*)$");
    QString name, value;

    pos = headerline.indexIn(line);
    if(pos != 0) return false;

    name = headerline.cap(1);
    value = headerline.cap(2);

    if(this->headers.contains(name))
        this->headers[name].append(value);
    else
        this->headers[name] = QList<QString>({value});

    return true;
}

QString HTTPParser::getMethod(){
    return this->method;
}

QString HTTPParser::getURL(){
    return this->url;
}

QString HTTPParser::getHTTPVersion(){
    return this->version;
}

QString HTTPParser::getData(){
    return this->data;
}

QHash<QString,QList<QString>> HTTPParser::getHeaders(){
    return this->headers;
}

bool HTTPParser::parseRequest(QString request){
    return this->parse(request);
}

void HTTPParser::prettyPrinter(){
    QHashIterator<QString,QList<QString>> i(this->getHeaders());
    while(i.hasNext()){
        i.next();
        for(QString value : i.value()){
            logger.info("Parsed Header: " + i.key().toUtf8().toStdString() + " -> " + value.toUtf8().toStdString());
        }
    }
    logger.info("Parsed Method: " + this->getMethod().toUtf8().toStdString());
    logger.info("Parsed HTTP Version: " + this->getHTTPVersion().toUtf8().toStdString());
    logger.info("Parsed URL: " + this->getURL().toUtf8().toStdString());
    logger.info("Parsed Data: \"" + this->getData().toUtf8().toStdString() + "\"");
}
