// ProxyGate main window - Source code.

// Includes:
#include "include/mainwindow.h"
#include "ui_mainwindow.h"

// Class methods:
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
                                          ui(new Ui::MainWindow),
                                          logger("Main") {
  // UI configuration:
  ui->setupUi(this);

  // Functionality configuration:
  start_server();

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

// Public methods:
int MainWindow::start_server() {

  server = new Server(server_port());     // Create the server.

  if(server->init() == 0) {
    server->moveToThread(&server_t);
    config_server_thread();
    server_t.start();
  }

  else {
    logger.error("Failed to initialize server!");
    return -1;
  }

  return 0;

}

// Private methods:
in_port_t MainWindow::server_port() {

  const QStringList args = QCoreApplication::arguments();
  in_port_t port_num;
  unsigned int arg_port_num;

  // Check for a specific port number:
  if(args.count() == 2) {
    arg_port_num = unsigned (args[1].toInt());

    // Valid port number:
    if(arg_port_num <= 65535)
      port_num = in_port_t (arg_port_num);

    // Invalid port number:
    else {
      port_num = DEFAULT_PORT;
      logger.warning("Invalid port number argument! Using default port instead.");
    }

  }

  // Else, just use the default:
  else
    port_num = DEFAULT_PORT;

  return port_num;

}

void MainWindow::config_server_thread() {

  // Configure server thread signals and slots:
  //connect(server, SIGNAL (error(QString)), this, SLOT (errorString(QString)));
  connect(&server_t, SIGNAL (started()), server, SLOT (run()));
  //connect(server, SIGNAL (finished()), &server_t, SLOT (quit()));
  //connect(server, SIGNAL (finished()), server, SLOT (deleteLater()));
  connect(&server_t, SIGNAL (finished()), &server_t, SLOT (deleteLater()));

}
