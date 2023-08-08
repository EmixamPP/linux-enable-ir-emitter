#include "logger.hpp"

#include <string>
#include <iostream>
using namespace std;

bool Logger::isDebugEnabled = false;

void Logger::enableDebug()
{
    Logger::isDebugEnabled = true;
}

void Logger::debug(const string &text)
{
    if (Logger::isDebugEnabled)
        cout << "DEBUG: " << text << endl;
}

void Logger::info(const string &text)
{
    cout << "INFO: " << text << endl;
}

void Logger::error(const string &text)
{
    cerr << "ERROR: " << text << endl;
}

void Logger::critical(const string &text)
{
    cerr << "CRITICAL: " << text << endl;
}