#ifndef LOGGER_HPP
#define LOGGER_HPP

class Logger
{
private:
    static bool isDebugEnabled;

public:
    Logger(Logger &other) = delete;
    void operator=(const Logger &) = delete;
    Logger() = delete;

    static void enableDebug();

    static void debug(const char *text);

    static void info(const char *text);

    static void error(const char *text);

    static void critical(const char *text);
};

#endif