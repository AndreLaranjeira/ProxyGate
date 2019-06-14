#ifndef SPIDER_H
#define SPIDER_H

#include <list>
#include <QString>
#include <sys/socket.h>
#include <netdb.h>
#include <QObject>
#include <QRegularExpression>

#include "include/socket.h"
#include "include/message_logger.h"
#include "include/httpparser.h"

#define SPIDER_TREE_DEPTH 3

class SpiderTree {
    private:
        list<SpiderTree> nodes;
        QString link;
        QString pp(int);
    public:
        SpiderTree(QString);
        void appendNode(SpiderTree);
        QString prettyPrint();
};

class Spider : public QObject {

    Q_OBJECT

    private:
    MessageLogger logger;


    int get(QString, QString *ret);
    int con(QString, int *);
    QStringList extract_links(QString);
    QString getAbsoluteLink(QString, QString);
    QString getURL(QString);
    QString getHost(QString);
    SpiderTree buildSpiderTree(QString, int, QStringList *);

    public:
        Spider();

    public slots:
        void execute(QString);

    signals:
        void updateSpiderTree(QString);
        void updateLog(QString);

};

#endif // SPIDER_H
