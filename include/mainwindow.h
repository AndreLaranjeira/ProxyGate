// Main window module - Header file.

// Header guard:
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Qt includes:
#include <QMainWindow>
#include <QObject>
#include <QThread>

// User includes:
#include "include/message_logger.h"
#include "include/server.h"

// Namespace:
namespace Ui {
  class MainWindow;
}

// Class headers:
class MainWindow : public QMainWindow {
  Q_OBJECT

  public:
    // Class methods:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // Methods:
    int start_server();

  private:
    // Classes:
    Ui::MainWindow *ui;
    MessageLogger logger;
    QThread server_t;
    Server *server;

};

#endif // MAINWINDOW_H
