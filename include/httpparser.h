#ifndef HTTPPARSER_H
#define HTTPPARSER_H

#include <QString>
#include <QList>
#include <iostream>
#include <QHash>

#include "include/message_logger.h"

// Enum that describes allowed states for Parser
typedef enum {
    COMMANDLINE,
    HEADERLINE,
    DATA
} ParserState;

// Typedef to abstract Headers hash
typedef QHash<QString,QList<QString>> Headers;

// HTTPParser class that implements parser for HTTP requests
class HTTPParser {
    private:
        // Private variables
        QString method;
        QString url;
        QString version;
        QString data;
        Headers headers;

        // Parser state
        ParserState state;

        // Message logger
        MessageLogger logger;

        // Private aux methods for parsing
        bool parseCommandLine(QString);
        bool parseHeaderLine(QString);
        bool parse(QString);
    public:
        // Constructor
        HTTPParser();

        // Getters
        QString getMethod();
        QString getHost();
        QString getURL();
        QString getHTTPVersion();
        QString getData();
        Headers getHeaders();

        // Parser
        bool parseRequest(QString);

        // PrettyPrinter method
        void prettyPrinter();

        // Converts parsed HTTP to QString
        QString toQString();
};

#endif // HTTPPARSER_H
