#include "logger.hpp"


bool Logger::isDebugEnabled = false;

void Logger::enableDebug()
{
    Logger::isDebugEnabled = true;
}
