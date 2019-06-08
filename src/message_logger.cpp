// Error logger module - Source code.

// Includes:
#include "include/message_logger.h"
#include "include/mainwindow.h"

extern MainWindow *w;

// Class methods:
MessageLogger::MessageLogger(std::string p_context) {
  context = p_context;
}

MessageLogger::~MessageLogger() {

}

void MessageLogger::connectMainWindow(){
    if(w != nullptr && !connected){
      connected = true;
      connect(this, SIGNAL(sendMessage(QString)), w, SLOT(logMessage(QString)));
    }
}

// Public methods:
void MessageLogger::error(string message) {
  cerr << context << ": " << "[Error] " << message << endl;
  connectMainWindow();
  emit sendMessage(QString::fromStdString(context + ": " + "[Error] " + message));
}

void MessageLogger::info(string message) {
  cout << context << ": " << "[Info] " << message << endl;
  connectMainWindow();
  emit sendMessage(QString::fromStdString(context + ": " + "[Info] " + message));
}

void MessageLogger::success(string message) {
  cout << context << ": " << "[Success] " << message << endl;
  connectMainWindow();
  emit sendMessage(QString::fromStdString(context + ": " + "[Success] " + message));
}

void MessageLogger::warning(string message) {
  cout << context << ": " << "[Warning] " << message << endl;
  connectMainWindow();
  emit sendMessage(QString::fromStdString(context + ": " + "[Warning] " + message));
}
