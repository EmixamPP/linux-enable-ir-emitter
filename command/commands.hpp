#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <string>
using namespace std;

#include "globals.hpp"

extern "C"
{
    ExitCode configure(const char* device, bool manual, unsigned emitters, unsigned negAnswerLimit, bool noGui);
    ExitCode delete_driver(const char* device);
    ExitCode run(const char* device);
    ExitCode test(const char* device);
    void enableDebug();
}

#endif
