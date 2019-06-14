// ProxyGate main window - Source code.

// Includes:
#include "include/mainwindow.h"
#include "ui_mainwindow.h"

// Class methods:
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
                                          ui(new Ui::MainWindow) {
  // UI configuration:
  ui->setupUi(this);

  // Functionality configuration:
  start_server();

  // Configure spider
  spider = new Spider;
  spider_t = new QThread;

  spider->moveToThread(spider_t);

  connect(this, SIGNAL(start_spider(QString)), spider, SLOT(execute(QString)));
  connect(spider, SIGNAL(updateSpiderTree(QString)), ui->spider_tree, SLOT(setText(QString)));
  connect(spider, SIGNAL(updateLog(QString)), ui->spider_log, SLOT(append(QString)));
  spider_t->start();

}

MainWindow::~MainWindow() {

  // Clean up the UI:
  delete ui;

  // Clean up the server thread:
  if(server_t->isRunning())
    server->stop();     // Stop the server. Signals and slots handle the rest!

  // Success message:
  //logger.success("Exited application!");

}

// Public methods:
int MainWindow::start_server() {

  server = new Server(server_port());
  server_t = new QThread;

  //connect(server, SIGNAL(errorMessage(QString)), this, SLOT(logMessage(QString)));

  if(server->init() == 0) {
    server->moveToThread(server_t);
    config_server_thread();
    server_t->start();
  }

  else {
    //logger.error("Failed to initialize server!");
    server->deleteLater();
    server_t->deleteLater();
    return -1;
  }

  return 0;

}

//void MainWindow::logMessage(QString message) {
//  QTextEdit *t = this->findChild<QTextEdit*>("logTextEdit");
//  t->append(message);
//}

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
      //logger.warning("Invalid port number argument! Using default port " + to_string(DEFAULT_PORT) + " instead.");
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

  // Configure server message logger:
  connect(server, SIGNAL (logMessage(QString)), ui->logTextEdit,
          SLOT (append(QString)));

  // Configure the client request to be displayed:
  connect(server, SIGNAL (client_request(QString)), ui->text_client,
          SLOT (setPlainText(QString)));

  // Configure the website request to be displayed:
  connect(server, SIGNAL (website_request(QString)), ui->text_website,
          SLOT (setPlainText(QString)));

  // Configure the gate to erase both requests displayed in text boxes:
  connect(server, SIGNAL (gate_opened()), ui->text_client, SLOT (clear()));
  connect(server, SIGNAL (gate_opened()), ui->text_website, SLOT (clear()));

  // The server thread should start the server:
  connect(server_t, SIGNAL (started()), server, SLOT (run()));

  // If the server run finishes, the server thread should quit:
  connect(server, SIGNAL (finished()), server_t, SLOT (quit()));

  // When the server finishes, schedule the server for deletion:
  connect(server, SIGNAL (finished()), server, SLOT (deleteLater()));

  // When the server thread finishes, schedule the server thread for deletion:
  connect(server_t, SIGNAL (finished()), server_t, SLOT (deleteLater()));
}

void MainWindow::on_button_gate_clicked() {
  server->open_gate();
}

void MainWindow::on_spider_push_clicked() {
  emit start_spider(ui->spider_host->text());
}

