// Main window module - Header file.

// Header guard:
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Qt includes:
#include <QMainWindow>
#include <QObject>
#include <QThread>
#include <QTextEdit>

// User includes:
//#include "include/message_logger.h"
#include "include/server.h"
#include "include/spider.h"

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

  public slots:
    //void logMessage(QString);

  private slots:
    void on_button_gate_clicked();
    void on_spider_push_clicked();

  signals:
    void start_spider(QString);

  private:
    // Classes:
    Ui::MainWindow *ui;
    //MessageLogger logger;
    QThread *server_t;
    Server *server;

    QThread *spider_t;
    Spider *spider;

    // Methods:
    in_port_t server_port();
    void config_server_thread();

};

#endif // MAINWINDOW_H
