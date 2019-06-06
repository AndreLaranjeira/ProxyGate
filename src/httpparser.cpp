#include "include/httpparser.h"

// Parser initializer
HTTPParser::HTTPParser() : state(COMMANDLINE), logger("HTTPParser"){
}

// Private method that receives a request and parses it
// Returns true if no error occoured and false if
// something went wrong

bool HTTPParser::parse(QString request){
    QList<QString> lines;

    // Split in \r\n
    lines = request.split(QRegExp("\r\n"));

    // Loop through lines
    for (QString line : lines) {

        // Take an action depending on state
        switch (state) {

            // CommandLine state is the first line to be read
            case COMMANDLINE:
                // ParseCommandLine method modifies class attributes
                if(!this->parseCommandLine(line)){
                   logger.error("Could not parse COMMAND LINE");
                   return false;
                }

                // Changes state immediately
                state = HEADERLINE;
            break;

            // HeaderLine state is to interpret line as a header
            case HEADERLINE:

                // Empty line means begin of data section
                if(line == ""){
                    state = DATA;
                }
                else {
                    // ParseHeaderLine modifies class attributes
                    if(!this->parseHeaderLine(line)){
                        logger.error("Could not parse HEADER LINE");
                        return false;
                    }
                }
            break;

            // Data state means to copy blindly the body into data
            case DATA:
                this->data += line;
            break;
        }
    }

    logger.info("Successfully parsed HTTP request");

    return true;
}

// This method parsers first line of a HTTP request
// It alters private members of class
// Returns false if could not match with expected expression
bool HTTPParser::parseCommandLine(QString line){
    int pos;

    // Regular Expression for command line
    QRegExp commandline("^(GET|HEAD|CONNECT|PUT|DELETE|POST|OPTIONS|TRACE|PATCH) ([:|\\/\\.\\\\-\\w_~/?\\#\\[\\]@!\\$\\^&'\\(\\)\\*\\+\\´,;=%{}]+) (HTTP\\/(?:\\d.\\d))$");
    QString method, url, version;

    // Try to match
    pos = commandline.indexIn(line);

    // Should match exactly the first character
    if(pos != 0) return false;

    // Set private variables
    this->method = commandline.cap(1);
    this->url = commandline.cap(2);
    this->version = commandline.cap(3);

    return true;
}

// This method parsers a header line of a HTTP request
// It alters private members of class
// Returns false if could not match with expected expression
bool HTTPParser::parseHeaderLine(QString line){
    int pos;

    // Regular Expression for Header Line
    QRegExp headerline("^([A-Za-z-]*): (.*)$");
    QString name, value;

    // Try to match
    pos = headerline.indexIn(line);
    if(pos != 0) return false;

    name = headerline.cap(1);
    value = headerline.cap(2);

    // Verify if hash already contains header name
    if(this->headers.contains(name))
        // Appends the value to QList
        // Used QList rather than simple a QString because the same header name
        // can appear multiple times with different values
        this->headers[name].append(value);
    else
        // Initialize a list with captured value
        this->headers[name] = QList<QString>({value});

    return true;
}

// Getter for method
QString HTTPParser::getMethod(){
    return this->method;
}

// Getter for host
QString HTTPParser::getHost(){
    return this->getHeaders()["Host"][0];
}

// Getter for url
QString HTTPParser::getURL(){
    return this->url;
}

// Getter for version
QString HTTPParser::getHTTPVersion(){
    return this->version;
}

// Getter for data
QString HTTPParser::getData(){
    return this->data;
}

// Getter for headers
Headers HTTPParser::getHeaders(){
    return this->headers;
}

// Public method that parsers a request
bool HTTPParser::parseRequest(QString request){
    return this->parse(request);
}

// Shows in screen beautified the parsed HTTP request
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

// Converts HTTPParser object into a QString request
QString HTTPParser::toQString(){
    QString ret;
    QHashIterator<QString,QList<QString>> i(this->getHeaders());

    // Renders first line
    ret += this->getMethod() + " " + this->getURL() + " " + this->getHTTPVersion() + "\r\n";

    // Renders each header line
    while(i.hasNext()){
        i.next();
        for(QString value : i.value()){
            ret += i.key() + ": " + value + "\r\n";
        }
    }

    // Renders empty line
    ret += "\r\n";

    // Renders data
    ret += this->getData();

    return ret;
}
