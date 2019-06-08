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
    HEADERLINE
} ParserState;

typedef struct HeaderBodyPair {
    QString header;
    char body[8192];
    ssize_t body_size;
} HeaderBodyPair;

// Typedef to abstract Headers hash
typedef QHash<QString,QList<QString>> Headers;

// HTTPParser class that implements parser for HTTP requests
class HTTPParser {
    private:
        // Private variables
        QString method;
        QString url;
        QString version;
        QString code;
        QString description;
        HeaderBodyPair splitted;
        Headers headers;

        // Parser state
        ParserState state;

        // Message logger
        MessageLogger logger;

        // Private aux methods for parsing
        HeaderBodyPair splitRequest(char *, ssize_t);
        bool parseCommandLine(QString);
        bool parseAnswerLine(QString);
        bool parseHeaderLine(QString);
        bool parse(char *, ssize_t);
    public:
        // Constructor
        HTTPParser();

        // Getters
        QString getMethod();
        QString getHost();
        QString getURL();
        QString getHTTPVersion();
        QString getCode();
        QString getDescription();
        char *getData();
        ssize_t getDataSize();
        Headers getHeaders();

        // Parser
        bool parseRequest(char *, ssize_t);

        // PrettyPrinter method
        void prettyPrinter();

        // Converts parsed HTTP to QString
        QString toQString();
};

#endif // HTTPPARSER_H
