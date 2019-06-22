// ProxyGate main window - Source code.

// Includes:
#include "include/mainwindow.h"
#include "ui_mainwindow.h"

// Class methods:
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
                                          ui(new Ui::MainWindow),
                                          logger("Main window") {
                                            
  // UI configuration:
  ui->setupUi(this);

  // Logger configuration:
  connect(&logger, SIGNAL (sendMessage(QString)), ui->logTextEdit,
          SLOT (append(QString)));

  // Create client hexedit
  text_client = new QHexEdit;
  text_client->setOverwriteMode(false);
  text_client->setDynamicBytesPerLine(true);
  ui->request_box->addWidget(text_client);

  // Create website hexedit
  text_website = new QHexEdit;
  text_website->setOverwriteMode(false);
  text_website->setDynamicBytesPerLine(true);
  ui->reply_box->addWidget(text_website);

  // Functionality configuration:
  start_server();
  start_tools();

}

MainWindow::~MainWindow() {

  // Clean up qhexedits
  delete text_client;
  delete text_website;

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

  // Initialize classes:
  server_t = new QThread;
  server = new Server(server_port());

  // If the server initializes, start the thread:
  if(server->init() == 0) {
    server->moveToThread(server_t);
    config_server_thread();
    server_t->start();
  }

  // Else, schedule the thread and the server for deletion:
  else {
    logger.error("Failed to initialize server!");
    server->deleteLater();
    server_t->deleteLater();
    return -1;
  }

  return 0;

}

void MainWindow::start_tools() {

  // Initialize classes:
  tools_t = new QThread;
  spider = new Spider;

  // Move classes to the thread:
  spider->moveToThread(tools_t);

  // Start the thread:
  config_tools_thread();
  tools_t->start();

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
      logger.warning("Invalid port number argument! Using default port " + to_string(DEFAULT_PORT) + " instead.");
    }

  }

  // Else, just use the default:
  else
    port_num = DEFAULT_PORT;

  return port_num;

}

void MainWindow::config_server_thread() {

  // Configure server thread signals and slots:

  // Configure server message logger:
  connect(server, SIGNAL (logMessage(QString)), ui->logTextEdit,
          SLOT (append(QString)));

  // Configure the client request body to be displayed:
  connect(server, SIGNAL (clientData(QString, QByteArray)), this,
          SLOT (setClientData(QString, QByteArray)));

  // Configure the website request to be displayed:
  connect(server, SIGNAL (websiteData(QString, QByteArray)), this,
          SLOT (setWebsiteData(QString, QByteArray)));

  // Configure the gate to erase both requests displayed in text boxes:
  connect(server, SIGNAL (gateOpened()), this, SLOT (clearClientData()));
  connect(server, SIGNAL (gateOpened()), this, SLOT (clearWebsiteData()));

  // The server thread should start the server:
  connect(server_t, SIGNAL (started()), server, SLOT (run()));

  // If the server run finishes, the server thread should quit:
  connect(server, SIGNAL (finished()), server_t, SLOT (quit()));

  // When the server finishes, schedule the server for deletion:
  connect(server, SIGNAL (finished()), server, SLOT (deleteLater()));

  // When the server thread finishes, schedule the server thread for deletion:
  connect(server_t, SIGNAL (finished()), server_t, SLOT (deleteLater()));

}

void MainWindow::config_tools_thread() {

  // Configure spider thread to run when user clicks the button:
  connect(this, SIGNAL(start_spider(QString)), spider, SLOT(execute(QString)));

  // Configure spider logger:
  connect(spider, SIGNAL(updateLog(QString)), ui->spider_log, SLOT(append(QString)));

  // Configure the spider host text field to receive updates from the server:
  // REFACTOR: Maybe we should check to see if a spider tree is running
  // before overwriting the host field.
  connect(server, SIGNAL (newHost(QString)), ui->spider_host, SLOT(setText(QString)));

  // Configure the spider output to be displayed:
  connect(spider, SIGNAL(updateSpiderTree(QString)), ui->spider_tree, SLOT(setText(QString)));

  // When the tools thread finishes, schedule the spider object for deletion:
  connect(tools_t, SIGNAL (finished()), spider, SLOT (deleteLater()));

  // When the tools thread finishes, schedule the tools thread for deletion:
  connect(tools_t, SIGNAL (finished()), tools_t, SLOT (deleteLater()));

}

// Private slots:
void MainWindow::on_button_gate_clicked() {
    // !TODO: Fix to add headers
  server->load_client_header(ui->request_headers->toPlainText(), text_client->data());
  server->load_website_header(ui->reply_headers->toPlainText(), text_website->data());
  server->open_gate();
}

void MainWindow::on_spider_push_clicked() {
  emit start_spider(ui->spider_host->text());
}

void MainWindow::setClientData(QString headers, QByteArray data){
    text_client->setData(data);
    ui->request_headers->setText(headers);
}

void MainWindow::setWebsiteData(QString headers, QByteArray data){
    text_website->setData(data);
    ui->reply_headers->setText(headers);
}

void MainWindow::clearClientData(){
    text_client->setData(QByteArray());
    ui->request_headers->clear();
}

void MainWindow::clearWebsiteData(){
    text_website->setData(QByteArray());
    ui->reply_headers->clear();
}
