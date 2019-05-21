// ProxyGate - Main function.

// Includes:
#include <QApplication>
#include <QThread>

// User includes:
#include "include/mainwindow.h"
#include "include/server.h"

// Main function:
int main(int argc, char *argv[]) {

  // Variable declaration:
  QApplication a(argc, argv);
  MainWindow w;
  Server *server = new Server();

  // Thread declaration:
  QThread *server_t = new QThread;

  // Server configuration:
  server->moveToThread(server_t);
  server_t->start();

  w.show();

  return a.exec();

}
