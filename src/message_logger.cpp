// Message logger module - Source code.

/**
 * @file message_logger.cpp
 * @brief Message logger module - Source code.
 *
 * The message logger module contains the implementation of a simple message
 * logger class that can be reused by any component on the project. This source
 * file contains the class method implementations for this module.
 *
 */

// Includes:
#include "include/message_logger.h"

// Class methods:

/**
 * @fn MessageLogger::MessageLogger(std::string p_context)
 * @brief Class constructor for the MessageLogger class.
 * @param p_context Context private variable for the MessageLogger.
 *
 * This constructor creates a new instance of the MessageLogger class. Each
 * instance has a p_context argument that configures the context private
 * variable of the class, used to identify the source of a MessageLogger
 * message.
 *
 */

MessageLogger::MessageLogger(std::string p_context) {
  context = p_context;
}

/**
 * @fn MessageLogger::~MessageLogger()
 * @brief Class destructor for the MessageLogger class.
 *
 * This destructor destroys an instance of the MessageLogger class. It is
 * currently empty!
 *
 */

MessageLogger::~MessageLogger() {

}

// Public methods:

/**
 * @fn void MessageLogger::error(string message)
 * @brief Method to log an error message.
 * @param message Error message to be sent by the logger.
 *
 * This method formats an error message using the MessageLogger context, the
 * 'Error' tag and the message argument. This error message is then emitted in
 * a sendMessage signal and outputed in cerr.
 *
 */

void MessageLogger::error(string message) {
  cerr << context << ": " << "[Error] " << message << endl;
  emit sendMessage(QString::fromStdString(context + ": " + "[Error] " + message));
}

/**
 * @fn void MessageLogger::info(string message)
 * @brief Method to log an information message.
 * @param message Message to be sent by the logger.
 *
 * This method formats an information message using the MessageLogger context,
 * the 'Info' tag and the message argument. This information message is then
 * emitted in a sendMessage signal and outputed in cout.
 *
 */

void MessageLogger::info(string message) {
  cout << context << ": " << "[Info] " << message << endl;
  emit sendMessage(QString::fromStdString(context + ": " + "[Info] " + message));
}

/**
 * @fn void MessageLogger::success(string message)
 * @brief Method to log a success message.
 * @param message Message to be sent by the logger.
 *
 * This method formats a success message using the MessageLogger context, the
 * 'Success' tag and the message argument. This success message is then emitted
 * in a sendMessage signal and outputed in cout.
 *
 */

void MessageLogger::success(string message) {
  cout << context << ": " << "[Success] " << message << endl;
  emit sendMessage(QString::fromStdString(context + ": " + "[Success] " + message));
}

/**
 * @fn void MessageLogger::warning(string message)
 * @brief Method to log a warning message.
 * @param message Message to be sent by the logger.
 *
 * This method formats a warning message using the MessageLogger context, the
 * 'Warning' tag and the message argument. This warning message is then emitted
 * in a sendMessage signal and outputed in cout.
 *
 */

void MessageLogger::warning(string message) {
  cout << context << ": " << "[Warning] " << message << endl;
  emit sendMessage(QString::fromStdString(context + ": " + "[Warning] " + message));
}
