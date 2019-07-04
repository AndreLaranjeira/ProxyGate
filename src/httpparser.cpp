#include "include/httpparser.h"

/**
 * @fn HTTPParser::HTTPParser()
 * @brief HTTPParser constructor
 *
 * Creates HTTP parser object setting initial state to COMMANDLINE and
 * connecting its logger to mainwindow
 *
 */
HTTPParser::HTTPParser() : state(COMMANDLINE), logger("HTTPParser"){
  // Connect message logger:
  connect(&logger, SIGNAL (sendMessage(QString)), this, SIGNAL (logMessage(QString)));
}

HTTPParser::~HTTPParser() {

}

/**
 * @fn HeaderBodyPair HTTPParser::splitRequest(char *request, size_t size)
 * @brief Splits header section from body section given an array of chars and its size
 * @param request Array of chars to be splitted
 * @param size Size of array of chars
 * @return Returns HeaderBodyPair struct with each field sets *
 */
HeaderBodyPair HTTPParser::splitRequest(char *request, size_t size){
    HeaderBodyPair ret;
    unsigned int headerEnd;

    ret.body_size = 0;

    for(headerEnd = 0 ; headerEnd + 3 < size ; headerEnd++ ){
        if(request[headerEnd]   == '\r' &&
           request[headerEnd+1] == '\n' &&
           request[headerEnd+2] == '\r' &&
           request[headerEnd+3] == '\n')
        break;
    }

    ret.header = QString::fromStdString(std::string(request, headerEnd));

    if(size > headerEnd + 4){
        ret.body_size = size - headerEnd - 4;
        memcpy(&(ret.body), &(request[headerEnd+4]), ret.body_size);
    }

    return ret;

}

/**
 * @fn bool HTTPParser::parse(char *request, ssize_t size)
 * @brief Private method that receives a request and parses it
 * @param request Array of chars to be parsed
 * @param size Size of array of chars
 * @return Returns true if no error occoured and false if not
 */
bool HTTPParser::parse(char *request, ssize_t size){
    QList<QString> lines;

    // Clean up variables:
    headers.clear();

    // Set initial state
    state = COMMANDLINE;

    // Split text part of body part
    this->splitted = splitRequest(request, static_cast<size_t>(size));

    // Split in \r\n
    lines = splitted.header.split(QRegExp("\r\n"));

    // Loop through lines
    for (QString line : lines) {

        // Take an action depending on state
        switch (state) {

            // CommandLine state is the first line to be read
            case COMMANDLINE:
                // ParseCommandLine method modifies class attributes
                if(!this->parseCL(line) && !this->parseAL(line)){
                   logger.error("Could not parse COMMAND LINE: \"" + line.toStdString() + "\"");
                   return false;
                }

                // Changes state immediately
                state = HEADERLINE;
            break;

            // HeaderLine state is to interpret line as a header
            case HEADERLINE:
                // ParseHeaderLine modifies class attributes
                if(!this->parseHL(line)){
                    logger.error("Could not parse HEADER LINE: \"" + line.toStdString() + "\"");
                    return false;
                }
            break;
        }
    }

    logger.info("Successfully parsed HTTP request");

    return true;
}

/**
 * @fn bool HTTPParser::validAnswerHeader(QString header)
 * @brief Verifies if an answer header string is valid
 * @param header QString containing the header
 * @return Returns true if valid, false if not
 *
 * A valid header is a header that ends with "\r\n\r\n", first
 * line is an answer line and the other lines are header lines
 */
bool HTTPParser::validAnswerHeader(QString header){

  QList<QString> lines;

  // Check to see if the header end is correct:
  if(!header.endsWith("\r\n\r\n"))
    return false;

  // Remove the header end:
  header.remove(header.size()-4, 4);

  // Split the remaining lines:
  lines = header.split(QRegExp("\r\n"));

  // The first line should be an answer line:
  if(!parseAnswerLine(lines.at(0), nullptr, nullptr, nullptr))
    return false;

  // The rest of the lines should be header lines:
  for(QString header_line : lines.mid(1))
    if(!parseHeaderLine(header_line, nullptr, nullptr))
      return false;

  return true;

}

/**
 * @fn bool HTTPParser::validRequestHeader(QString header)
 * @brief Verifies if an request header string is valid
 * @param header QString containing the header
 * @return Returns true if valid, false if not
 *
 * A valid header is a header that ends with "\r\n\r\n", first
 * line is an request line and the other lines are header lines
 */
bool HTTPParser::validRequestHeader(QString header) {

    QList<QString> lines;

    // Check to see if the header end is correct:
    if(!header.endsWith("\r\n\r\n"))
      return false;

    // Remove the header end:
    header.remove(header.size()-4, 4);

    // Split the remaining lines:
    lines = header.split(QRegExp("\r\n"));

    // The first line should be a commmand line:
    if(!parseCommandLine(lines.at(0), nullptr, nullptr, nullptr))
      return false;

    // The rest of the lines should be header lines:
    for(QString header_line : lines.mid(1)) {
      if(!parseHeaderLine(header_line, nullptr, nullptr))
        return false;
    }

    return true;
}

/**
 * @fn bool HTTPParser::parseCommandLine(QString line, QString *method, QString *url, QString *version)
 * @brief Given a line try to parse a command line
 * @param line The line to be parsed
 * @return Returns true if parsed ok
 * @return method by reference
 * @return url by reference
 * @return version by reference
 */
bool HTTPParser::parseCommandLine(QString line, QString *method, QString *url, QString *version){
    int pos;

    // Regular Expression for command line
    QRegExp commandline("^(GET|HEAD|CONNECT|PUT|DELETE|POST|OPTIONS|TRACE|PATCH) ([:|\\/\\.\\\\-\\w_~/?\\#\\[\\]@!\\$\\^&'\\(\\)\\*\\+\\´,;=%{}]+) (HTTP\\/(?:\\d.\\d))$");

    // Try to match
    pos = commandline.indexIn(line);

    // Should match exactly the first character
    if(pos != 0) return false;

    // Set return
    if(method != nullptr)
        *method = commandline.cap(1);

    if(url != nullptr)
        *url = commandline.cap(2);

    if(version != nullptr)
        *version = commandline.cap(3);

    return true;
}

/**
 * @fn bool HTTPParser::parseCL(QString line)
 * @brief This method parsers first line of a HTTP request, it alters private members of class
 * @param line The line to be parsed
 * @return Returns false if could not match with expected expression
 */
bool HTTPParser::parseCL(QString line){
    // Parse command line and set private variables
    return parseCommandLine(line, &this->method, &this->url, &this->version);
}

/**
 * @fn bool HTTPParser::parseAnswerLine(QString line, QString *version, QString *code, QString *description)
 * @brief Given a line try to parse a answer line
 * @param line The line to be parsed
 * @return Returns true if parsed ok
 * @return version by reference
 * @return code by reference
 * @return description by reference
 */
bool HTTPParser::parseAnswerLine(QString line, QString *version, QString *code, QString *description){
    int pos;

    // Regular Expression for command line
    // HTTP/1.1 403 Forbidden
    QRegExp commandline("^(HTTP\\/(?:\\d.\\d)) (\\d{3}) (.*)$");

    // Try to match
    pos = commandline.indexIn(line);

    // Should match exactly the first character
    if(pos != 0) return false;

    // Set variables
    if(version != nullptr)
        *version = commandline.cap(1);

    if(version != nullptr)
        *code = commandline.cap(2);

    if(version != nullptr)
        *description = commandline.cap(3);

    return true;
}

/**
 * @fn bool HTTPParser::parseAL(QString line)
 * @brief This method parsers first line of a HTTP request, it alters private members of class
 * @param line The line to be parsed
 * @return Returns false if could not match with expected expression, true if could
 */
bool HTTPParser::parseAL(QString line){
    // Parsers answer line and set private variables
    return parseAnswerLine(line, &this->version, &this->code, &this->description);
}

/**
 * @fn bool HTTPParser::parseHeaderLine(QString line, QString *name, QString *value)
 * @brief This method parsers a header line of a HTTP request
 * @param line The line to be parsed
 * @return Returns false if could not match with expected expression
 * @return name key of header line by reference
 * @return value of header line by reference
 */
bool HTTPParser::parseHeaderLine(QString line, QString *name, QString *value){
    int pos;

    // Regular Expression for Header Line
    QRegExp headerline("^([A-Za-z-0-9]*): (.*)$");

    // Try to match
    pos = headerline.indexIn(line);
    if(pos != 0) return false;

    if(name != nullptr)
        *name = headerline.cap(1);

    if(value != nullptr)
        *value = headerline.cap(2);

    return true;
}

/**
 * @fn bool HTTPParser::parseHL(QString line)
 * @brief This method parsers a header line of a HTTP request, it alters private members of class
 * @param line The line to be parsed
 * @return Returns false if could not match with expected expression
 */
bool HTTPParser::parseHL(QString line){
    QString name, value;

    // Parsers header line
    if(!this->parseHeaderLine(line, &name, &value))
        return false;

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

/**
 * @fn QString HTTPParser::getMethod()
 * @brief Getter for http method
 * @return Returns QString containing method
 */
QString HTTPParser::getMethod(){
    return this->method;
}

/**
 * @fn QString HTTPParser::getHost()
 * @brief Getter for http host
 * @return Returns QString containing host
 */
QString HTTPParser::getHost(){
    if(this->headers.contains("Host"))
        return this->getHeaders()["Host"][0];
    else return "";
}

/**
 * @fn QString HTTPParser::getURL()
 * @brief Getter for http url
 * @return Returns QString containing url
 */
QString HTTPParser::getURL(){
    return this->url;
}

/**
 * @fn QString HTTPParser::getHTTPVersion()
 * @brief Getter for http version
 * @return Returns QString containing version
 */
QString HTTPParser::getHTTPVersion(){
    return this->version;
}

/**
 * @fn QString HTTPParser::getCode()
 * @brief Getter for http code
 * @return Returns QString containing code
 */
QString HTTPParser::getCode(){
    return this->code;
}

/**
 * @fn QString HTTPParser::getDescription()
 * @brief Getter for http description
 * @return Returns QString containing description
 */
QString HTTPParser::getDescription(){
    return this->description;
}

/**
 * @fn QString HTTPParser::getData()
 * @brief Getter for http data section
 * @return Returns char array containing raw data
 */
char *HTTPParser::getData(){
    return this->splitted.body;
}

/**
 * @fn QString HTTPParser::getDataSize()
 * @return Returns size of data section
 */
size_t HTTPParser::getDataSize(){
    return this->splitted.body_size;
}

/**
 * @fn QString HTTPParser::getHeaders()
 * @brief Getter for headers
 * @return Returns headers hashmap
 */
Headers HTTPParser::getHeaders(){
    return this->headers;
}

/**
 * @fn QString HTTPParser::getHeadersSize()
 * @return Returns number header size in bytes
 */
int HTTPParser::getHeadersSize(){
    return this->splitted.header.size();
}

/**
 * @fn bool HTTPParser::parseRequest(char *request, ssize_t size)
 * @brief Public method that parsers a request
 * @param request array of chars with request to be parsed
 * @param size size of array of chars with request
 * @return Returns true if parsed ok, false otherwise
 */
bool HTTPParser::parseRequest(char *request, ssize_t size){
    return this->parse(request, size);
}


/**
* @fn void HTTPParser::prettyPrinter()
* @brief Shows in stdout beautified the parsed HTTP request
*/
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
    for(size_t i=0 ; i<this->getDataSize(); i++){
//        if(this->getData()[i] == '\0'){
//            std::cout << "END OF STRING FOUND, MAYBE BINARY DATA" << endl;
//            //break;
//        }
        std::cout << this->getData()[i] << " (" << (static_cast<int>(this->getData()[i])) << ")" << endl;
    }
    cout << endl;
}

/**
* @fn QString HTTPParser::answerHeaderToQString()
* @return Returns as a QString the header with answer line
*/
QString HTTPParser::answerHeaderToQString() {
  QString ret;

  // Renders the answer line:
  ret += this->getHTTPVersion() + " " + this->getCode() + " " + this->getDescription() + "\r\n";
  ret += headerFieldsToQString();

  return ret;

}

/**
* @fn QString HTTPParser::headerFieldsToQString()
* @brief Converts header of HTTPParser object into a QString
* @return Returns as a QString the header lines
*/
QString HTTPParser::headerFieldsToQString(){
    QString ret;
    QHashIterator<QString,QList<QString>> i(this->getHeaders());

    // Renders each header line
    while(i.hasNext()){
        i.next();
        for(QString value : i.value()){
            ret += i.key() + ": " + value + "\r\n";
        }
    }

    // Renders empty line
    ret += "\r\n";

    return ret;
}

/**
* @fn QString HTTPParser::requestHeaderToQString()
* @return Returns as a QString the header with request line
*/
QString HTTPParser::requestHeaderToQString() {
  QString ret;

  // Renders the answer line:
  ret += this->getMethod() + " " + this->getURL() + " " + this->getHTTPVersion() + "\r\n";
  ret += headerFieldsToQString();

  return ret;

}

/**
* @fn QByteArray HTTPParser::requestBuffer()
* @return raw request buffer as QByteArray
*/
QByteArray HTTPParser::requestBuffer(){
    QByteArray ret;
    ret.append(this->requestHeaderToQString());
    ret.append(this->getData(), static_cast<int>(this->getDataSize()));
    return ret;
}

/**
* @fn QByteArray HTTPParser::answerBuffer()
* @return raw answer buffer as QByteArray
*/
QByteArray HTTPParser::answerBuffer(){
    QByteArray ret;
    ret.append(this->answerHeaderToQString());
    ret.append(this->getData(), static_cast<int>(this->getDataSize()));
    return ret;
}

/**
* @fn void HTTPParser::updateContentLength()
* @brief Recomputes content-length based on body size
*/
void HTTPParser::updateContentLength(){
    if(this->headers.contains("Content-Length")){
        this->headers["Content-Length"][0] = QString::fromStdString(std::to_string(this->splitted.body_size));
    }
}
