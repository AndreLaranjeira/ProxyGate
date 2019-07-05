// Main window module - Header file.

/**
 * @file mainwindow.h
 * @brief Main window module - Header file.
 *
 * The main window module contains the implementation of the MainWindow class,
 * which is responsible for the UI and thread configuration of the application.
 * This header file contains a header guard, library includes and the class
 * headers for this module.
 *
 */

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

/**
 * @class MainWindow
 * @brief Main window class.
 *
 * The MainWindow class represents the main window seem in the application. It
 * is responsible for updating displays, receiving user inputs and handling the
 * application's threads.
 *
 * When the application is closed, this class is deleted.
 *
 */

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
    void start_spider(QString);           /**< Signals a Spider start call. */
    void start_dumper(QString, QString);  /**< Signals a Dumper start call. */

  private:
    // Classes:
    Ui::MainWindow *ui;     /**< User interface. */
    MessageLogger logger;   /**< MessageLogger used by the MainWindow. */
    QThread *server_t;      /**< Server thread. */
    Server *server;         /**< Server class used in the server thread. */
    QHexEdit *text_client;  /**< Client data hexadecimal edit sub-window. */
    QHexEdit *text_website; /**< Website data hexadecimal edit sub-window. */

    QThread *tools_t;       /**< Tools thread. */
    SpiderDumper *spider;   /**< SpiderDumper class used in the tools thread. */

    // Methods:
    in_port_t server_port();
    void config_server_thread();
    void config_tools_thread();

};

#endif // MAINWINDOW_H
