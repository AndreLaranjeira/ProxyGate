#ifndef SPIDER_H
#define SPIDER_H

#include <iostream>
#include <fstream>
#include <list>
#include <QString>
#include <sys/socket.h>
#include <netdb.h>
#include <QObject>
#include <QRegularExpression>
#include <QDir>

#include "include/socket.h"
#include "include/message_logger.h"
#include "include/httpparser.h"

#define SPIDER_TREE_DEPTH 4

class SpiderTree {
    private:
        list<SpiderTree> nodes;
        QString link;
        QString pp(unsigned int);
    public:
        SpiderTree(QString);
        void appendNode(SpiderTree);
        QString prettyPrint();
};

class SpiderDumper : public QObject {

    Q_OBJECT

    private:
    MessageLogger logger;


    int get(QString, QByteArray *ret);
    int con(QString, int *);
    QStringList extract_links(QString);
    QStringList extract_references(QString);
    QString getAbsoluteLink(QString, QString);
    QString getURL(QString);
    QString getHost(QString);
    SpiderTree buildSpiderTree(QString, QString, int, QStringList *);
    void dump(QString, QString, int, QStringList *);
    bool sameHost(QString, QString);
    QString removeWWW(QString);
    QString removeSquare(QString);

    public:
        SpiderDumper();

    public slots:
        void spider(QString);
        void dumper(QString);

    signals:
        void updateSpiderTree(QString);
        void updateLog(QString);

};

#endif // SPIDER_H
