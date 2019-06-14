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

class Spider : public QObject {

    Q_OBJECT

    private:
    MessageLogger logger;


    int get(QString, QString *ret);
    int connect(QString, int *);
    QStringList extract_links(QString);
    QString getAbsoluteLink(QString, QString);
    QString getURL(QString);
    QString getHost(QString);

    public:
        Spider();

    public slots:
        void execute(QString);



};

#endif // SPIDER_H
