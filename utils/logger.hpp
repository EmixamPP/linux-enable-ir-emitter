#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
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

    Logger(Logger &&other) = delete;

    static void enableDebug();

    template <typename... Args>
    static void debug(Args... args)
    {
        if (Logger::isDebugEnabled)
        {
            cout << "DEBUG:";
            ((cout << " " << args), ...);
            cout << endl;
        }
    }

    template <typename... Args>
    static void info(Args... args)
    {
        cout << "INFO:";
        ((cout << " " << args), ...);
        cout << endl;
    }

    template <typename... Args>
    static void error(Args... args)
    {
        cout << "ERROR:";
        ((cout << " " << args), ...);
        cout << endl;
    }

    template <typename... Args>
    [[noreturn]] static void critical(int code, Args... args)
    {
        cout << "CRITICAL:";
        ((cout << " " << args), ...);
        cout << endl;
        exit(code);
    }
};

#endif
