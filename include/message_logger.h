// Message logger module - Header file.

// Header guard:
#ifndef MESSAGE_LOGGER_H
#define MESSAGE_LOGGER_H

// Includes:
#include <iostream>
#include <string>

// Namespace:
using namespace std;

// Class headers:
class MessageLogger {

  public:
    // Class methods:
    MessageLogger(string);
    ~MessageLogger();

    // Methods:
    void error(string);
    void info(string);
    void success(string);
    void warning(string);

  private:
    // Variables:
    string context;

};

#endif // MESSAGE_LOGGER_H

/* TODO list:
 *  - Adapt this module to a Qt module with Qstring and a main window output.
 *
 */
