#pragma once

#include "camera/camera.hpp"
#include "utils/logger.hpp"
#include "configuration.hpp"

#include <filesystem>
#include <memory>
#include <regex>
#include <signal.h>
using namespace std;

const regex DEVICE_PATTERN("/dev/video[0-9]+");

enum ExitCode
{
    SUCCESS = 0,
    FAILURE = 1,
    FILE_DESCRIPTOR_ERROR = 126,
    ROOT_REQUIRED = 2
};

/**
 * @brief Catch ctrl-c signal one time.
 */
inline void CatchCtrlC()
{
    auto handler = [](int signal)
    {
        if (signal == SIGINT)
        {
            static int ctrlc_counter = 0;
            ++ctrlc_counter;
            if (ctrlc_counter == 2)
                exit(ExitCode::FAILURE);
            cout << " Ctrl-c again if you really want to, be careful this could break the camera." << endl;
        }
    };

    signal(SIGINT, std::move(handler));
}

/**
 * @brief Creates a Camera or AutoCamera object.
 *
 * @tparam T type which inherited from `Camera`
 * @param device path to the camera
 * @param width of the capture resolution
 * @param height of the capture resolution
 * @param no_gui
 * 
 * @throw CameraException if the device is invalid
 * 
 * @return a smart pointer to the created object
 */
template <typename T>
inline shared_ptr<T> CreateCamera(const string &device, int width, int height, bool no_gui = false)
{
    shared_ptr<T> camera;

    if (device.empty())
    {
        // find a greyscale camera
        auto devices = T::Devices();
        for (const auto &device : devices)
        {
            Logger::debug("Checking if", device, "is a greyscale camera.");
            try
            {
                camera = make_shared<T>(device, width, height);
                if (camera->is_gray_scale())
                {
                    Logger::debug(device, "is a greyscale camera.");
                    break;
                }
            }
            catch (const CameraException &)
            { // ignore
            }
        }

        if (!camera)
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
    ExitCode run(const char *device);
    ExitCode test(const char *device, int width, int height);
    ExitCode tweak(const char *device, int width, int height);
    void enable_debug();
}
