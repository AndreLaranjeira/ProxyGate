/**
 * @file httpparser.h
 * @brief HTTP parser module - Header file.
 *
 * This module implements HTTP parser, it creates a data structure
 * containing a hashmap for each header, the method of request, response
 * code, url and description. This module offers access methods for
 * those fields and renders back the HTTP data as QByteArray
 *
 */

#ifndef HTTPPARSER_H
#define HTTPPARSER_H

#include <QString>
#include <QList>
#include <iostream>
#include <QHash>
#include <QObject>

#include "include/message_logger.h"

/**
 * @macro HTTP_BUFFER_SIZE
 * @brief Maximum size for every request and response on system
 */
#define HTTP_BUFFER_SIZE 1048576

/**
 * @enum ParserState
 * @brief Enum that describes allowed states for Parser
 */
typedef enum {
    COMMANDLINE, /**< Parser should parse first HTTP line. */
    HEADERLINE /**< Parse should parse a header line. */
} ParserState;

/**
 * @struct HeaderBodyPair
 * @brief Internal HTTP Parser struct that splits headers section from body section
 */
typedef struct HeaderBodyPair {
    QString header; /**< Header string */
    char body[HTTP_BUFFER_SIZE+1]; /**< Raw body data. */
    size_t body_size; /**< Raw body data size. */
} HeaderBodyPair;

/**
 * @typedef Headers
 * @brief Hashmap that takes header name and gives a list of values
 */
typedef QHash<QString,QList<QString>> Headers;

/**
 * @class HTTPParser
 * @brief HTTPParser class that implements parser for HTTP requests
 */
class HTTPParser : public QObject {
    Q_OBJECT

    private:
        // Private variables
        QString method; /**< Stores HTTP method such as GET, POST, PUT, etc. */
        QString url; /**< Stores HTTP url. */
        QString version; /**< Stores HTTP version. */
        QString code; /**< Stores response code. */
        QString description; /**< Stores response description. */
        HeaderBodyPair splitted; /**< Stores splitted pair: header,body. */
        Headers headers; /**< Hashmap with keys as header name and value a list of header values. */

        // Parser state
        ParserState state; /**< Current parser state. */

        // Message logger
        MessageLogger logger; /**< Parser logger. */

        // Private aux methods for parsing
        HeaderBodyPair splitRequest(char *, size_t);
        bool parseCL(QString);
        bool parseHL(QString);
        bool parseAL(QString);
        bool parse(char *, ssize_t);

    public:
        // Constructor
        HTTPParser();
        ~HTTPParser();

        // Getters
        QString getMethod();
        QString getHost();
        QString getURL();
        QString getHTTPVersion();
        QString getCode();
        QString getDescription();
        char *getData();
        size_t getDataSize();
        int getHeadersSize();
        Headers getHeaders();

        // Parser
        bool parseRequest(char *, ssize_t);

        // PrettyPrinter method
        void prettyPrinter();

        // Converts parsed HTTP headers to QString
        QString answerHeaderToQString();
        QString headerFieldsToQString();
        QString requestHeaderToQString();

        // Converts parsed HTTP to QByteArray
        QByteArray requestBuffer();
        QByteArray answerBuffer();

        // Parse a header line
        bool parseHeaderLine(QString, QString *, QString *);

        // Parse a command line
        bool parseCommandLine(QString, QString *, QString *, QString *);

        // Parse an answer line
        bool parseAnswerLine(QString, QString *, QString *, QString *);

        // Verifies if a header line from REQUEST is valid
        bool validRequestHeader(QString);

        // Verifies if a header line from ANSWER is valid
        bool validAnswerHeader(QString);

        // Updates content length based on body size
        void updateContentLength();

        // Verifies if a header line from REQUEST is valid
        inline bool validRequestHeaderLine(QString);

        // Verifies if a header line from ANSWER is valid
        inline bool validAnswerHeaderLine(QString);

    signals:
        void logMessage(QString);

};

#endif // HTTPPARSER_H
