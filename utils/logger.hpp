#pragma once

#include <iostream>
using namespace std;

class Logger
{
private:
    static bool debug_;

public:
    Logger() = delete;

    ~Logger() = default;

    Logger(Logger &other) = delete;

    void operator=(const Logger &) = delete;

    Logger &operator=(Logger &&other) = delete;

    Logger(Logger &&other) = delete;

    static void enable_debug();

    template <typename... Args>
    static void debug(Args ...args)
    {
        if (Logger::debug_)
        {
            cout << "DEBUG:";
            ((cout << " " << args), ...);
            cout << endl << flush;
        }
    }

    template <typename... Args>
    static void info(Args ...args)
    {
        cout << "INFO:";
        ((cout << " " << args), ...);
        cout << endl << flush;
    }

    template <typename... Args>
    static void info_no_endl(Args ...args)
    {
        cout << "INFO:";
        ((cout << " " << args), ...);
        cout << flush;
    }

    template <typename... Args>
    static void warning(Args ...args)
    {
        cout << "WARNING:";
        ((cout << " " << args), ...);
        cout << endl << flush;
    }

    template <typename... Args>
    static void error(Args ...args)
    {
        cout << "ERROR:";
        ((cout << " " << args), ...);
        cout << endl << flush;
    }

    template <typename... Args>
    [[noreturn]] static void critical(int code, Args ...args)
    {
        cout << "CRITICAL:";
        ((cout << " " << args), ...);
        cout << endl << flush;
        exit(code);
    }
};
