#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
using namespace std;

class Logger
{
private:
    static bool isDebugEnabled;

public:  
    Logger() = delete;

    ~Logger() = default;

    Logger(Logger &other) = delete;

    void operator=(const Logger &) = delete;

    Logger &operator=(Logger &&other) = delete;

    Logger(Logger && other) = delete;

    static void enableDebug();

    static void debug(const string &text);

    static void info(const string &text);

    static void error(const string &text);

    static void critical(const string &text);
};

#endif