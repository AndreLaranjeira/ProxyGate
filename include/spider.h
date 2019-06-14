#ifndef SPIDER_H
#define SPIDER_H

#include <list>
#include <QString>
#include <sys/socket.h>
#include <netdb.h>
#include <QObject>

#include "include/socket.h"
#include "include/message_logger.h"
#include "include/httpparser.h"

class Spider : public QObject {

    Q_OBJECT

    private:
    MessageLogger logger;


    int get(QString, QString *ret);
    int connect(QString, int *);

    public:
        Spider();

    public slots:
        void execute(QString);



};

#endif // SPIDER_H
