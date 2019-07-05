// ProxyGate - Main function.

/**
 * @file main.cpp
 * @brief Main file.
 *
 * This main file is a simple default Qt main file that starts the Qt
 * application.
 *
 */

// Qt includes:
#include <QApplication>

// User includes:
#include "include/mainwindow.h"

// Main function:

/**
 * @fn int main(int argc, char *argv[])
 * @brief Main function.
 * @param argc Number of arguments.
 * @param argv Program arguments.
 * @return Program return code.
 *
 * This main function is a simple default Qt main function that starts the Qt
 * application.
 *
 */

int main(int argc, char *argv[]) {

  // Class declarations:
  QApplication a(argc, argv);     // This declaration should always come first!

  MainWindow w;

  // Show the main window contents:
  w.show();

  // Execute the application:
  return a.exec();

}
