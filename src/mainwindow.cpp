// ProxyGate main window - Source code.

// Includes:
#include "include/mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
                                          ui(new Ui::MainWindow),
                                          logger("Main") {
  // UI configuration:
  ui->setupUi(this);

  // Server configuration:
  if(this->start_server() == 0) {
    server->moveToThread(&server_t);
    //connect(server, SIGNAL (error(QString)), this, SLOT (errorString(QString)));
    connect(&server_t, SIGNAL (started()), server, SLOT (run()));
    //connect(server, SIGNAL (finished()), &server_t, SLOT (quit()));
    //connect(server, SIGNAL (finished()), server, SLOT (deleteLater()));
    connect(&server_t, SIGNAL (finished()), &server_t, SLOT (deleteLater()));
    server_t.start();
  }

  else
   logger.error("Failed to initialize server!");

}

MainWindow::~MainWindow() {

  // Clean up the UI:
  delete ui;

  // Clean up the server thread:
  if(server_t.isRunning()) {
    server->stop();     // Stop the server.
    server_t.quit();    // Quit the thread.
    server_t.wait();    // Wait for the thread to finish.
  }

  delete server;

  // Success message:
  logger.success("Application finished!");

}

int MainWindow::start_server() {

  // Variable declarations:
  const QStringList args = QCoreApplication::arguments();
  in_port_t port_num;
  unsigned int arg_port_num;

  // Check for a specific port number:
  if(args.count() == 2) {
    arg_port_num = unsigned (args[1].toInt());

    if(arg_port_num <= 65535)
      port_num = in_port_t (arg_port_num);

    else {
      port_num = DEFAULT_PORT;
      logger.warning("Invalid port number argument! Using default port instead.");
    }

  }

  else
    port_num = DEFAULT_PORT;

  // Server configuration:
  server = new Server(port_num);

  if(server->init() != 0)
    return -1;

  return 0;

}
