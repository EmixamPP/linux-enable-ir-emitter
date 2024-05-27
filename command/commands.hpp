#pragma once

#include "camera/camera.hpp"
#include "utils/logger.hpp"
#include "configuration.hpp"

#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <regex>
#include <string>
using namespace std;

enum ExitCode
{
    SUCCESS = 0,
    FAILURE = 1,
    ROOT_REQUIRED = 2,
    FILE_DESCRIPTOR_ERROR = 126,
};

/**
 * @brief Creates a Camera or AutoCamera object.
 *
 * @tparam T type which inherited from `Camera`
 * @param device path of the camera, nothing for automatic detection
 * @param width of the capture resolution
 * @param height of the capture resolution
 * @param no_gui
 *
 * @throw CameraException if the device is invalid
 *
 * @return a smart pointer to the created object
 */
template <typename T>
inline shared_ptr<T> CreateCamera(const optional<string> &device, int width, int height, bool no_gui = false)
{
    shared_ptr<T> camera;

    if (!device.has_value())
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
        camera = make_shared<T>(device.value(), width, height);

    if (no_gui)
        camera->disable_gui();

    return camera;
}

ExitCode configure(const optional<string> &device, int width, int height, bool manual, unsigned emitters, unsigned neg_answer_limit, bool no_gui);

ExitCode run(const optional<string> &device, int width, int height);

ExitCode test(const optional<string> &device, int width, int height);

ExitCode tweak(const optional<string> &device, int width, int height);
