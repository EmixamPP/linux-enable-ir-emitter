#include "logger.hpp"


bool Logger::debug_ = false;

void Logger::enable_debug()
{
    Logger::debug_ = true;
}
