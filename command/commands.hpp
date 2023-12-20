#pragma once

#include "globals.hpp"

extern "C"
{
    ExitCode configure(const char* device, bool manual, unsigned emitters, unsigned negAnswerLimit, bool noGui);
    ExitCode delete_config(const char* device);
    ExitCode run(const char* device);
    ExitCode test(const char* device);
    ExitCode tweak(const char* device);
    void enableDebug();
}
