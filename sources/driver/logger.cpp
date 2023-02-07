#include "logger.hpp"

#include <string>
#include <cstdlib>
#include <fstream>
#include <iostream>
using namespace std;

bool Logger::isDebugEnabled = false;

void Logger::enableDebug()
{
    Logger::isDebugEnabled = true;
}

void Logger::debug(const char *text)
{
    if (Logger::isDebugEnabled)
        cout << "DEBUG: " << text << endl;
}

void Logger::info(const char *text)
{
    cout << "INFO: " << text << endl;
}

void Logger::error(const char *text)
{
    cerr << "ERROR: " << text << endl;
}

void Logger::critical(const char *text)
{
    cerr << "CRITICAL: " << text << endl;
}