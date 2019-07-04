// Message logger module - Header file.

/**
 * @file message_logger.h
 * @brief Message logger module - Header file.
 *
 * The message logger module contains the implementation of a simple message
 * logger class that can be reused by any component on the project. This header
 * file contains a header guard, library includes and the class headers for
 * this module
 *
 */

// Header guard:
#ifndef MESSAGE_LOGGER_H
#define MESSAGE_LOGGER_H

// Includes:
#include <iostream>
#include <string>
#include <QObject>

// Namespace:
using namespace std;

// Class headers:

/**
 * @class MessageLogger
 * @brief A simple message logger class used by other classes.
 *
 * This class implements a simple message logger that was made to give other
 * classes a way to log messages in a window of the application.
 *
 * To use the MessageLogger in your class, you must ensure your class inherits
 * from the QObject class and configure the MessageLogger.
 *
 * First, create the MessageLogger (it is preferable for it to be private)
 * and a signal that transmits a QString in your class header:
 *
 *     class YourClass : public QObject {
 *         Q_OBJECT
 *
 *         // Add these to your class:
 *
 *         signals:
 *             void logMessage(QString);
 *
 *         private:
 *             MessageLogger my_logger;
 *
 *         // Obviously, your class should have other methods and variables
 *         // beside these (like a constructor)!
 *
 *     }
 *
 * Then, add the MessageLogger initialization to your class constructor:
 *
 *     YourClass::YourClass(parameters) : my_logger("Context name") {
 *
 *         // Connect the message logger signal to your signal:
 *         connect(&my_logger, SIGNAL (sendMessage(QString)), this,
                   SIGNAL (logMessage(QString)));
 *
 *         // The rest of your constructor code goes here!
 *
 *     }
 *
 * And you are all set! Now all messages generated using the message logger
 * will emit the signal logMessage(QString), which should be treated by the
 * MainWindow class or some other qualified class. In case you do NOT want
 * YourClass to just propagate a signal, feel free to remove the logMessage
 * signal from the class header and to treat the sendMessage signal from
 * my_logger in whatever way you want to!
 *
 */

class MessageLogger : public QObject {

  Q_OBJECT

  public:
    // Class methods:
    MessageLogger(string);
    ~MessageLogger();

    // Methods:
    void error(string);
    void info(string);
    void success(string);
    void warning(string);

    signals:
    void sendMessage(QString);  /**< Signal to send a log message. */

  private:
    // Variables:
    string context;   /**< Context where the logger was created. */

};

#endif // MESSAGE_LOGGER_H
