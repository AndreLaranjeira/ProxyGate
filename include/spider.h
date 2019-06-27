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

#define SPIDER_TREE_DEPTH 2

class SpiderTree {
    private:
        list<SpiderTree> nodes;
        QString link;
        QString pp(unsigned int);
        QByteArray data;
        QString contentType;
    public:
        SpiderTree(QString);
        void appendNode(SpiderTree);
        list<SpiderTree> *getNodes();
        QString getLink();
        QString prettyPrint();
        void setData(QByteArray);
        QByteArray getData();
        void setContentType(QString);
        QString getContentType();
};

class SpiderDumper : public QObject {

    Q_OBJECT

    private:
    MessageLogger logger;


    int get(QString, QByteArray *, QString *);
    int con(QString, int *);
    QStringList extract_links(QString);
    QStringList extract_references(QString);
    QString getAbsoluteLink(QString, QString);
    QString getURL(QString);
    QString getURL_relative(QString, QString);
    QString getHost(QString);
    SpiderTree buildSpiderTree(QString);
    SpiderTree buildSpiderTree(QString, bool);
    QByteArray buildSpiderTreeRecursive(SpiderTree *, QString, QString, int, QStringList *, bool);
    void dump(SpiderTree, QString);
    void dumpRecursive(SpiderTree, QString);
    bool sameHost(QString, QString);
    QString removeWWW(QString);
    QString removeSquare(QString);
    QString fix_references(QString, QString);
    QString buildBackDir(int);
    QString getFileName(QString);
    QString getFolderName(QString);
    QString getFileExtension(QString);
    void saveToFile(QString, QString, QByteArray);


    public:
        SpiderDumper();

    public slots:
        void spider(QString);
        void dumper(QString, QString);

    signals:
        void updateSpiderTree(QString);
        void updateLog(QString);

};

#endif // SPIDER_H
