#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <string>
using namespace std;

#include "globals.hpp"

extern "C"
{
    ExitCode configure(const char* device, bool manual, int emitters, int negAnswerLimit);
    ExitCode delete_driver(const char* device);
    ExitCode run(const char* device);
    ExitCode test(const char* device);
    void enableDebug();
}

#endif
