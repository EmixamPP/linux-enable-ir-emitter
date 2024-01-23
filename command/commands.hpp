#pragma once

#include "globals.hpp"
#include "utils/logger.hpp"

#include <memory>
using namespace std;

template <typename T>
shared_ptr<T> MakeCamera(const string &device, int width, int height, bool no_gui = false)
{
    shared_ptr<T> camera;

    if (device.empty())
    {
        camera = T::FindGrayscaleCamera(width, height);
        if (camera == nullptr)
            Logger::critical(ExitCode::FAILURE, "No infrared camera has been found.");
    }
    else
        camera = make_shared<T>(device, width, height);

    if (no_gui)
        camera->disable_gui();

    return camera;
}

extern "C"
{
    ExitCode configure(const char *device, int width, int height, bool manual, unsigned emitters, unsigned neg_answer_limit, bool no_gui);
    ExitCode delete_config(const char *device);
    ExitCode run(const char *device);
    ExitCode test(const char *device, int width, int height);
    ExitCode tweak(const char *device, int width, int height);
    void enable_debug();
}
