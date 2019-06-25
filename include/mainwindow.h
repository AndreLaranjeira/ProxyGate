// Main window module - Header file.

// Header guard:
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Qt includes:
#include <QMainWindow>
#include <QObject>
#include <QThread>
#include <QTextEdit>
#include <QFileDialog>

// User includes:
#include "include/message_logger.h"
#include "include/server.h"
#include "include/spider.h"
#include "include/qhexedit/qhexedit.h"

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
    void start_tools();

  private slots:
    void on_button_gate_clicked();
    void on_spider_push_clicked();
    void on_dumper_push_clicked();
    void setClientData(QString, QByteArray);
    void setWebsiteData(QString, QByteArray);
    void clearClientData();
    void clearWebsiteData();

  signals:
    void start_spider(QString);
    void start_dumper(QString, QString);

  private:
    // Classes:
    Ui::MainWindow *ui;
    MessageLogger logger;
    QThread *server_t;
    Server *server;
    QHexEdit *text_client;
    QHexEdit *text_website;


    QThread *tools_t;
    SpiderDumper *spider;

    // Methods:
    in_port_t server_port();
    void config_server_thread();
    void config_tools_thread();

};

#endif // MAINWINDOW_H
