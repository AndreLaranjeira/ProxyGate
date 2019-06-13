// ProxyGate - Main function.

// Qt includes:
#include <QApplication>

// User includes:
#include "include/mainwindow.h"

// Main function:
int main(int argc, char *argv[]) {

  // Class declarations:
  QApplication a(argc, argv);     // This declaration should always come first!

  MainWindow w;

  // Show the main window contents:
  w.show();

  // Execute the application:
  return a.exec();

}
