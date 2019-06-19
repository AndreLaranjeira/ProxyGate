#ifndef HTTPPARSER_H
#define HTTPPARSER_H

#include <QString>
#include <QList>
#include <iostream>
#include <QHash>
#include <QObject>

#include "include/message_logger.h"
#define HTTP_BUFFER_SIZE 1048576

// Enum that describes allowed states for Parser
typedef enum {
    COMMANDLINE,
    HEADERLINE
} ParserState;

typedef struct HeaderBodyPair {
    QString header;
    char body[HTTP_BUFFER_SIZE+1];
    size_t body_size;
} HeaderBodyPair;

// Typedef to abstract Headers hash
typedef QHash<QString,QList<QString>> Headers;

// HTTPParser class that implements parser for HTTP requests
class HTTPParser : public QObject {
    Q_OBJECT

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
        HeaderBodyPair splitRequest(char *, size_t);
        bool parseCommandLine(QString);
        bool parseAnswerLine(QString);
        bool parseHeaderLine(QString);
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
        Headers getHeaders();

        // Parser
        bool parseRequest(char *, ssize_t);

        // PrettyPrinter method
        void prettyPrinter();

        // Converts parsed HTTP headers to QString
        QString headerToQstring();

    signals:
        void logMessage(QString);

};

#endif // HTTPPARSER_H
