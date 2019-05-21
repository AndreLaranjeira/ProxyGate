// Server module - Header file.

// Header guard:
#ifndef SERVER_H
#define SERVER_H

// Includes:
#include <QObject>

// Namespace:
using namespace std;

// Class headers:
class Server : public QObject {

  Q_OBJECT

  // Methods:
  public:
  Server();
  ~Server();

  public slots:

  signals:

};

#endif // SERVER_H
