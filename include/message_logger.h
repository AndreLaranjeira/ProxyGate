// Message logger module - Header file.

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
    void sendMessage(QString);

  private:
    // Variables:
    string context;

};

#endif // MESSAGE_LOGGER_H

/* TODO list:
 *  - Adapt this module to a Qt module with Qstring and a main window output.
 *
 */
