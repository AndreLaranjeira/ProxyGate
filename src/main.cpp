// ProxyGate - Main function.

// Qt includes:
#include <QApplication>

// User includes:
#include "include/mainwindow.h"

MainWindow *w = nullptr;

// Main function:
int main(int argc, char *argv[]) {

  // Class declarations:
  QApplication a(argc, argv);     // This declaration should always come first!

  MainWindow win;
  w = &win;

  // Show the main window contents:
  win.show();

  // Execute the application:
  return a.exec();

}
