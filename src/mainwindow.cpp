// ProxyGate main window - Source code.

// Includes:
#include "include/mainwindow.h"
#include "ui_mainwindow.h"

// Class methods:
/**
 * @fn MainWindow::MainWindow(QWidget *parent)
 * @brief MainWindow constructor for MainWindow class
 * @param parent A QT parent widget
 *
 * This constructor creates main window logger, instanciates Ui class,
 * creates some qhexedit textboxes that can't be created statically and
 * configures server and tools
 *
 */
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

/**
 * @fn MainWindow::~MainWindow()
 * @brief MainWindow destructor for MainWindow class
 *
 * Deallocates qhexedits and Ui, cleans up server thread and
 * schedule server thread to stop
 *
 */
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

/**
 * @fn int MainWindow::start_server()
 * @brief Starts the server thread.
 * @return Returns 0 when the successfully executed and -1 if an error occurs.
 *
 * This method starts the server thread and the Server functionalities of the
 * application. If successful, a new thread is created with a Server class
 * running in it.
 *
 */

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

/**
 * @fn void MainWindow::start_tools()
 * @brief Starts the tools thread.
 * @return Returns 0 when the successfully executed.
 *
 * This method starts the tools thread and the SpiderDump functionalities of
 * the application. If successful, a new thread is created with a SpiderDumper
 * class running in it.
 *
 */

void MainWindow::start_tools() {

  // Initialize classes:
  tools_t = new QThread;
  spider = new SpiderDumper;

  // Move classes to the thread:
  spider->moveToThread(tools_t);

  // Start the thread:
  config_tools_thread();
  tools_t->start();

}

// Private methods:

/**
 * @fn in_port_t MainWindow::server_port()
 * @brief Method to determine the Server class port number.
 * @returns Returns the port number used by the Server class.
 *
 * This method receives the program arguments and uses it to determine the port
 * number used by the Server class. If the user provided a valid port number as
 * a program argument, it is used by the Server. Else, we use the default port
 * number specified in the Server module.
 *
 */

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

/**
 * @fn void MainWindow::config_server_thread()
 * @brief Method to configure the server thread signals and slots.
 *
 * This method configures the server thread connections using the Qt signals
 * and slots system.
 *
 */

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

/**
 * @fn void MainWindow::config_tools_thread()
 * @brief Method to configure the tools thread signals and slots.
 *
 * This method configures the tools thread connections using the Qt signals
 * and slots system.
 *
 */

void MainWindow::config_tools_thread() {

  // Configure spider thread to run when user clicks the button:
  connect(this, SIGNAL(start_spider(QString)), spider, SLOT(spider(QString)));

  // Configure spider thread to run dumper when user clicks the button:
  connect(this, SIGNAL(start_dumper(QString, QString)), spider, SLOT(dumper(QString, QString)));

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

/**
 * @fn void MainWindow::on_button_gate_clicked()
 * @brief This function is executed when button_gate is clicked
 *
 * When button gate is clicked it sends to server the requests and replies
 * stored on textboxes and qhexedits. After that it sets open gate internal server
 * flag with a call to server->open_gate()
 *
 */
void MainWindow::on_button_gate_clicked() {
    // !TODO: Fix to add headers
  server->load_client_request(ui->request_headers->toPlainText(), text_client->data());
  server->load_website_request(ui->reply_headers->toPlainText(), text_website->data());
  server->open_gate();
}

/**
 * @fn void MainWindow::on_spider_push_clicked()
 * @brief This function is executed when spider button is clicked
 *
 * Send a signal to SpiderDumper class with host to get links from and
 * build spider tree
 *
 */
void MainWindow::on_spider_push_clicked() {
  emit start_spider(ui->spider_host->text());
}

/**
 * @fn void MainWindow::on_dumper_push_clicked()
 * @brief This function is executed when dumper button is clicked
 *
 * Pops up a file dialog, that will tell where to save dumped files.
 * After that it emits a signal to SpiderDumer with host and selected
 * directory asking to begin dump process.
 *
 */
void MainWindow::on_dumper_push_clicked() {
  QString dir = QFileDialog::getExistingDirectory(this, tr("Escolha uma pasta"), ".", QFileDialog::ShowDirsOnly);
  emit start_dumper(ui->spider_host->text(), dir);
}

/**
 * @fn void MainWindow::setClientData(QString headers, QByteArray data)
 * @brief This is a slot that updates textbox with header data and
 * qhexedit with body data on client side.
 * @param headers QString with headers to be inserted on client textbox
 * @param data Raw data to be inserted on client qhexedit
 *
 */
void MainWindow::setClientData(QString headers, QByteArray data){
    text_client->setData(data);
    ui->request_headers->setText(headers);
}


/**
 * @fn void MainWindow::setWebsiteData(QString headers, QByteArray data)
 * @brief This is a slot that updates textbox with header data and
 * qhexedit with body data on website side.
 * @param headers QString with headers to be inserted on website textbox
 * @param data Raw data to be inserted on website qhexedit
 *
 */
void MainWindow::setWebsiteData(QString headers, QByteArray data){
    text_website->setData(data);
    ui->reply_headers->setText(headers);
}


/**
 * @fn void MainWindow::clearClientData()
 * @brief This is a slot that clears content from header textbox and qhexedit on client side *
 */
void MainWindow::clearClientData(){
    text_client->setData(QByteArray());
    ui->request_headers->clear();
}

/**
 * @fn void MainWindow::clearWebsiteData()
 * @brief This is a slot that clears content from header textbox and qhexedit on website side *
 */
void MainWindow::clearWebsiteData(){
    text_website->setData(QByteArray());
    ui->reply_headers->clear();
}
