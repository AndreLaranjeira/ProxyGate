#ifndef HTTPPARSER_H
#define HTTPPARSER_H

#include <QString>
#include <QList>
#include <iostream>
#include <QHash>

#include "include/message_logger.h"

typedef enum {
    COMMANDLINE,
    HEADERLINE,
    DATA
} ParserState;

class HTTPParser {
    private:
        QString method;
        QString url;
        QString version;
        QString data;
        QHash<QString, QList<QString>> headers;
        ParserState state;
        MessageLogger logger;
        bool parseCommandLine(QString);
        bool parseHeaderLine(QString);
        bool parse(QString);
    public:
        HTTPParser();
        QString getMethod();
        QString getURL();
        QString getHTTPVersion();
        QString getData();
        bool parseRequest(QString);
        void prettyPrinter();
        QHash<QString,QList<QString>> getHeaders();
};

#endif // HTTPPARSER_H
