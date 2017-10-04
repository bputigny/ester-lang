#ifndef LOG_H
#define LOG_H

#include "termcolor.hpp"

#include <iostream>
#include <cstdlib>
#include <string>
#include <ctime>

///
/// \brief Used to report an error: print an error message and exit the program,
/// returning EXIT_FAILURE
///
#define error(msg) { std::cerr              \
    << termcolor::bold << termcolor::red    \
    << "[ERROR]: "                          \
    << termcolor::reset                     \
    << termcolor::bold                      \
    << __FILE__ << ":" << __LINE__ << ", "  \
    << termcolor::reset                     \
    << "in function "                       \
    << termcolor::bold                      \
    << __func__ << ": "                     \
    << termcolor::reset                     \
    << termcolor::bold << termcolor::red    \
    << (msg) << '\n'                        \
    << termcolor::reset;                    \
    std::exit(EXIT_FAILURE);                \
}

namespace log {

    /*
    std::string str_time() {
        time_t rawtime;
        struct tm *timeinfo;
        char str[20];

        time(&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(str, 19, "%T ", timeinfo);
        return std::string(str);
    }
    */

    /// \brief used to log messages to standard output
    std::ostream& log();

    /// \brief used to log messages to standard error output
    ///
    /// example: log::warn() << "this is a warning\n";
    std::ostream& warn();

    /// \brief used to log messages to standard error output
    std::ostream& err();
}

#endif
