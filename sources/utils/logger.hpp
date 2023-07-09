#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
using namespace std;

class Logger
{
private:
    static bool isDebugEnabled;

public:
    Logger(Logger &other) = delete;
    void operator=(const Logger &) = delete;
    Logger() = delete;

    static void enableDebug();

    static void debug(string text);

    static void info(string text);

    static void error(string text);

    static void critical(string text);
};

#endif