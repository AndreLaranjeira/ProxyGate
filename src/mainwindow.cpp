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
  if(server_t->isRunning())
    server->stop();     // Stop the server. Signals and slots handle the rest!

  // Success message:
  logger.success("Exited application!");

}

// Public methods:
int MainWindow::start_server() {

  server = new Server(server_port());
  server_t = new QThread;

  if(server->init() == 0) {
    server->moveToThread(server_t);
    config_server_thread();
    server_t->start();
  }

  else {
    logger.error("Failed to initialize server!");
    server->deleteLater();
    server_t->deleteLater();
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

  // If there is an error, signal the Main Window:
  //connect(server, SIGNAL (error(QString)), this, SLOT (errorString(QString)));

  // The server thread should start the server:
  connect(server_t, SIGNAL (started()), server, SLOT (run()));

  // If the server run finishes, the server thread should quit:
  connect(server, SIGNAL (finished()), server_t, SLOT (quit()));

  // When the server finishes, schedule the server for deletion:
  connect(server, SIGNAL (finished()), server, SLOT (deleteLater()));

  // When the server thread finishes, schedule the server thread for deletion:
  connect(server_t, SIGNAL (finished()), server_t, SLOT (deleteLater()));

}
