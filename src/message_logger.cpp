// Error logger module - Source code.

// Includes:
#include "include/message_logger.h"
#include "include/mainwindow.h"
#include <QTextEdit>

// Class methods:
MessageLogger::MessageLogger(string p_context) {
  context = p_context;
}

MessageLogger::~MessageLogger() {

}

// Public methods:
void MessageLogger::error(string message) {
  cerr << context << ": " << "[Error] " << message << endl;
}

void MessageLogger::info(string message) {
  cout << context << ": " << "[Info] " << message << endl;
}

void MessageLogger::success(string message) {
  cout << context << ": " << "[Success] " << message << endl;
}

void MessageLogger::warning(string message) {
  cout << context << ": " << "[Warning] " << message << endl;
}
